//
//  MOD_squish.c
//  bf_modifiers
//
//  Created by claire on 9/16/22.
//

#include "BLT_translation.h"

#include "DNA_defaults.h"
#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_object_types.h"
#include "DNA_screen_types.h"
#include "DNA_camera_types.h"
#include "DNA_scene_types.h"

#include "DEG_depsgraph_query.h"

#include "RNA_access.h"
#include "RNA_prototypes.h"

#include "BKE_modifier.h"
#include "BKE_camera.h"

#include "UI_resources.h"

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

    /* initData */ NULL,
    /* requiredDataMask */ NULL,
    /* freeData */ NULL,
    /* isDisabled */ NULL,
    /* updateDepsgraph */ NULL,
    /* dependsOnTime */ NULL,
    /* dependsOnNormals */ NULL,
    /* foreachIDLink */ NULL,
    /* foreachTexLink */ NULL,
    /* freeRuntimeData */ NULL,
    /* panelRegister */ NULL,
    /* blendWrite */ NULL,
    /* blendRead */ NULL,
};
