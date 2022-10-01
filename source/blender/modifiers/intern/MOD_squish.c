//
//  MOD_squish.c
//  bf_modifiers
//
//  Created by claire on 9/16/22.
//

#include "BLI_math.h" // for some reason need this include for MEMCPY_STRUCT_AFTER
#include "BLI_utildefines.h"
#include "BLT_translation.h"

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

static void initData(ModifierData *md)
{
  SquishModifierData *smd = (SquishModifierData *)md;

  BLI_assert(MEMCMP_STRUCT_AFTER_IS_ZERO(smd, modifier));

  MEMCPY_STRUCT_AFTER(smd, DNA_struct_default_get(SquishModifierData), modifier);
}

static void updateDepsgraph(ModifierData *md, const ModifierUpdateDepsgraphContext *ctx)
{
  SquishModifierData *smd = (SquishModifierData *)md;
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
    Mesh *mesh_src = NULL;
    struct Scene *scene = DEG_get_input_scene(ctx->depsgraph);
    Object* cam = (Object*)scene->camera; // not getting camera data this time, just the object
    
    Object* ob = ctx->object;
    
    // matrix transform method
    
    float vertexTransform[4][4];
    unit_m4(vertexTransform);
    
    float camObjDiff[3];
    copy_v3_v3(camObjDiff, cam->loc);
    sub_v3_v3(camObjDiff, ob->loc);
    
    float rotateAngle = atan2f(camObjDiff[0], camObjDiff[1]);
    char rotateAxis = 'Z';
    float groundVector[2] = {camObjDiff[0], camObjDiff[1]};
    float groundVectorDist = len_v2(groundVector);
    float rotateAngleUp = atan2f(camObjDiff[2], groundVectorDist) * -1;
    char rotateAxisUp = 'X';
    float scaleVectorY[3] = {1.0f, smd->factor, 1.0f};
    
    // order is backwards because of matrix multiplication
    
    rotate_m4(vertexTransform, rotateAxis, -rotateAngle);
    rotate_m4(vertexTransform, rotateAxisUp, -rotateAngleUp);
    rescale_m4(vertexTransform, scaleVectorY);
    rotate_m4(vertexTransform, rotateAxisUp, rotateAngleUp);
    rotate_m4(vertexTransform, rotateAxis, rotateAngle);
    
    
    for (int i = 0; i < verts_num; i++) {
        float obCoor[3]; // store variable for processing coordinates
        
        copy_v3_v3(obCoor, vertexCos[i]); // this is important keep this
        
        mul_m4_v3(vertexTransform, obCoor);
        
//        // camera vector method (vertex addition method, slow)
//
//        // convert vertex to world space
//        mul_m4_v3(ob->obmat, obCoor);
//
//        // process vertexes in world space
//        float* camPos = cam->loc;
//        float* obLoc = ob->loc;
//        float camObjVec[3]; // vector between camera and center of object
//        copy_v3_v3(camObjVec, camPos);
//        sub_v3_v3(camObjVec, obLoc);
//        normalize_v3(camObjVec);
//
//        float vertexProject[3];
//        project_v3_v3v3(vertexProject, obCoor, camObjVec);
//
//        mul_v3_fl(vertexProject, smd->factor - 1.0f);
//
//        add_v3_v3(obCoor, vertexProject);
//
//        // convert back to local space
//        float iobMat[4][4];
//        invert_m4_m4(iobMat, ob->obmat);
//        mul_m4_v3(iobMat, obCoor);
        
            
//        // sphere projection method
//
//        // convert to world space
//        copy_v3_v3(obCoor, vertexCos[i]);
//        mul_m4_v3(ob->obmat, obCoor);
//
//        // process vertexes in world space
//        float* camPos = cam->loc;
//        float* obLoc = ob->loc;
//        float camObjVec[3]; // vector between camera and center of object
//        copy_v3_v3(camObjVec, camPos);
//        sub_v3_v3(camObjVec, obLoc);
//        float camObjectDist = len_v3(camObjVec);
//
//        float camVertexVec[3]; // vector between camera and individual vertex
//        copy_v3_v3(camVertexVec, obCoor);
//        sub_v3_v3(camVertexVec, camPos);
//        float camVertexDist = len_v3(camVertexVec);
//
//        float distFromSphere = camObjectDist - camVertexDist;
//        float vecToSphere[3];
//        copy_v3_v3(vecToSphere, camVertexVec);
//        normalize_v3(vecToSphere);
//        mul_v3_fl(vecToSphere, distFromSphere);
//
//        mul_v3_fl(vecToSphere, smd->factor);
//
//        add_v3_v3(obCoor, vecToSphere);
//
//        // convert to local space
//        float iobMat[4][4];
//        invert_m4_m4(iobMat, ob->obmat);
//        mul_m4_v3(iobMat, obCoor);
        
        // apply obCoor
        copy_v3_v3(vertexCos[i], obCoor);
    }
}

static void panel_draw(const bContext *UNUSED(C), Panel *panel)
{
  uiLayout *layout = panel->layout;

  PointerRNA ob_ptr;
  PointerRNA *ptr = modifier_panel_get_property_pointers(panel, &ob_ptr);
    
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
