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

static void deformVerts(ModifierData *md,
                        const ModifierEvalContext *ctx,
                        struct Mesh *mesh,
                        float (*vertexCos)[3],
                        int verts_num)
{
    SimpleDeformModifierData *sdmd = (SimpleDeformModifierData *)md;
    Mesh *mesh_src = NULL;
    struct Scene *scene = DEG_get_input_scene(ctx->depsgraph);
    Camera* cam = (Camera*)scene->camera;
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
    /* updateDepsgraph */ NULL,
    /* dependsOnTime */ NULL,
    /* dependsOnNormals */ NULL,
    /* foreachIDLink */ NULL,
    /* foreachTexLink */ NULL,
    /* freeRuntimeData */ NULL,
    /* panelRegister */ panelRegister,
    /* blendWrite */ NULL,
    /* blendRead */ NULL,
};
