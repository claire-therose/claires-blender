//
//  MOD_squish.c
//  bf_modifiers
//
//  Created by claire on 9/16/22.
//

#include "BLI_math.h" // for some reason need this include for MEMCPY_STRUCT_AFTER
#include "BLI_utildefines.h"
#include "BLT_translation.h"
#include "BLI_task.h"

#include "DNA_defaults.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_object_types.h"
#include "DNA_screen_types.h"
#include "DNA_camera_types.h"
#include "DNA_scene_types.h"

#include "BKE_modifier.h"
#include "BKE_camera.h"
#include "BKE_screen.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "RNA_access.h"
#include "RNA_prototypes.h"

#include "DEG_depsgraph_query.h"

#include "MOD_ui_common.h"

ALIGN_STRUCT struct SquishUserData {
    float factor;
    char mode;
    float (*vertexCos)[3];
    const SpaceTransform *camSpaceTransform;
    float camObjDist;
    float vertexTransform[4][4];
};

static void scaleCoorToSphere(float co[3], float sphereRadius, float factor)
{
    float coorDistance = len_v3(co);
    float coCopy[3];
    copy_v3_v3(coCopy, co);
    float distancePercentage = sphereRadius / coorDistance;
    mul_v3_fl(coCopy, distancePercentage);
    interp_v3_v3v3(co, coCopy, co, factor);
}

static void simple_helper(void *__restrict userdata,
                          const int iter,
                          const TaskParallelTLS *__restrict UNUSED(tls))
{
    const struct SquishUserData *curr_squish_data = userdata;
    
    /* copy vertex data */
    float co[3];
    copy_v3_v3(co, curr_squish_data->vertexCos[iter]);
    
    if (curr_squish_data->camSpaceTransform) {
      BLI_space_transform_apply(curr_squish_data->camSpaceTransform, co);
    }
    
    switch(curr_squish_data->mode) {
        case MOD_SQUISH_MODE_CAMERA:
            mul_m4_v3(curr_squish_data->vertexTransform, co);
            break;
        case MOD_SQUISH_MODE_INTERNAL:
            scaleCoorToSphere(co, curr_squish_data->camObjDist, curr_squish_data->factor);
            break;
    }
    
    if (curr_squish_data->camSpaceTransform) {
      BLI_space_transform_invert(curr_squish_data->camSpaceTransform, co);
    }
    
    /* set vertex data back */
    copy_v3_v3(curr_squish_data->vertexCos[iter], co);
}

static void initData(ModifierData *md)
{
  SquishModifierData *smd = (SquishModifierData *)md;

  BLI_assert(MEMCMP_STRUCT_AFTER_IS_ZERO(smd, modifier));

  MEMCPY_STRUCT_AFTER(smd, DNA_struct_default_get(SquishModifierData), modifier);
}

static void updateDepsgraph(ModifierData *md, const ModifierUpdateDepsgraphContext *ctx)
{
  DEG_add_object_relation(
    ctx->node, ctx->scene->camera, DEG_OB_COMP_TRANSFORM, "Squish Modifier");
  DEG_add_modifier_to_transform_relation(ctx->node, "Squish Modifier");
}

static void deformVerts(ModifierData *md,
                        const ModifierEvalContext *ctx,
                        struct Mesh *mesh,
                        float (*vertexCos)[3],
                        int verts_num)
{
    SquishModifierData *smd = (SquishModifierData *)md;
    struct Scene *scene = DEG_get_input_scene(ctx->depsgraph);
    Object* cam = (Object*)scene->camera; // not getting camera data this time, just the object
    
    Object* ob = ctx->object;
    
    /* set up data for vertex modification */
    SpaceTransform *camSpaceTransform = NULL, tmp_transf;
    camSpaceTransform = &tmp_transf;
    BLI_SPACE_TRANSFORM_SETUP(camSpaceTransform, ob, cam);
    
    float currentPosition[3] = {0.0f, 0.0f, 0.0f};
    BLI_space_transform_apply(camSpaceTransform, currentPosition);
    
    float vertexTransform[4][4];
    unit_m4(vertexTransform);
    
    float directionScale[3] = {1.0f, 1.0f, smd->factor};
    rescale_m4(vertexTransform, directionScale);
    transform_pivot_set_m4(vertexTransform, currentPosition);
    
    float* camPos = cam->loc;
    float* obLoc = ob->loc;
    float camObjVec[3];
    copy_v3_v3(camObjVec, camPos);
    sub_v3_v3(camObjVec, obLoc);
    float camObjDist = len_v3(camObjVec);

    /* copied from MOD_simpledeform.c */
    
    /* Build our data. */
    /* vertex transform is a bit ugly, but makes the data shared, which is
     much better for efficiency */
    const struct SquishUserData squish_pool_data = {
        .factor = smd->factor,
        .mode = smd->mode,
        .vertexCos = vertexCos,
        .camSpaceTransform = camSpaceTransform,
        .camObjDist = camObjDist,
        .vertexTransform[0][0] = vertexTransform[0][0],
        .vertexTransform[0][1] = vertexTransform[0][1],
        .vertexTransform[0][2] = vertexTransform[0][2],
        .vertexTransform[0][3] = vertexTransform[0][3],
        .vertexTransform[1][0] = vertexTransform[1][0],
        .vertexTransform[1][1] = vertexTransform[1][1],
        .vertexTransform[1][2] = vertexTransform[1][2],
        .vertexTransform[1][3] = vertexTransform[1][3],
        .vertexTransform[2][0] = vertexTransform[2][0],
        .vertexTransform[2][1] = vertexTransform[2][1],
        .vertexTransform[2][2] = vertexTransform[2][2],
        .vertexTransform[2][3] = vertexTransform[2][3],
        .vertexTransform[3][0] = vertexTransform[3][0],
        .vertexTransform[3][1] = vertexTransform[3][1],
        .vertexTransform[3][2] = vertexTransform[3][2],
        .vertexTransform[3][3] = vertexTransform[3][3]
    };
    /* Do deformation. */
    TaskParallelSettings settings;
    BLI_parallel_range_settings_defaults(&settings);
    BLI_task_parallel_range(0, verts_num, (void *)&squish_pool_data, simple_helper, &settings);
}

static void panel_draw(const bContext *UNUSED(C), Panel *panel)
{
  uiLayout *layout = panel->layout;

  PointerRNA ob_ptr;
  PointerRNA *ptr = modifier_panel_get_property_pointers(panel, &ob_ptr);
    
  uiItemR(layout, ptr, "deform_method", UI_ITEM_R_EXPAND, NULL, ICON_NONE);
    
  uiLayoutSetPropSep(layout, true);
    
  uiItemR(layout, ptr, "factor", 0, NULL, ICON_NONE);

  modifier_panel_end(layout, ptr);
}

static void panelRegister(ARegionType *region_type)
{
  modifier_panel_register(region_type, eModifierType_Squish, panel_draw);
}

ModifierTypeInfo modifierType_Squish = {
    /* name */ N_("Squish"),
    /* structName */ "SquishModifierData",
    /* structSize */ sizeof(SquishModifierData),
    /* srna */ &RNA_SquishModifier,
    /* type */ eModifierTypeType_OnlyDeform,

    /* flags */ eModifierTypeFlag_AcceptsMesh,
    /* icon */ ICON_FULLSCREEN_EXIT,

    /* copyData */ BKE_modifier_copydata_generic,

    /* deformVerts */ deformVerts,
    /* deformMatrices */ NULL,
    /* deformVertsEM */ NULL,
    /* deformMatricesEM */ NULL,
    /* modifyMesh */ NULL,
    /* modifyGeometrySet */ NULL,

    /* initData */ initData,
    /* requiredDataMask */ NULL,
    /* freeData */ NULL,
    /* isDisabled */ NULL,
    /* updateDepsgraph */ updateDepsgraph,
    /* dependsOnTime */ NULL,
    /* dependsOnNormals */ NULL,
    /* foreachIDLink */ NULL,
    /* foreachTexLink */ NULL,
    /* freeRuntimeData */ NULL,
    /* panelRegister */ panelRegister,
    /* blendWrite */ NULL,
    /* blendRead */ NULL,
};
