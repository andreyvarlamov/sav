#ifdef TEMPLATE_EXPORTS
#define TEMPLATE_FUNC extern "C" __declspec(dllexport)
#else
#define TEMPLATE_FUNC extern "C" __declspec(dllimport)
#endif

#include "savt_game.h"

TEMPLATE_FUNC entity
Template_PumiceWall()
#ifdef TEMPLATE_EXPORTS
{
    entity Template = {};
    Template.Type = ENTITY_STATIC;
    Template.Color = VA_SLATEGRAY;
    Template.Glyph = 11 + 16*13;
    Template.Condition = 100.0f;
    Template.Name = "Pumice Wall";
    Template.Description = "Wall made of soft pumice rock.";
    SetFlags(&Template.Flags, ENTITY_IS_BLOCKING | ENTITY_IS_OPAQUE);
    return Template;
}
#else
;
#endif

TEMPLATE_FUNC entity
Template_Player()
#ifdef TEMPLATE_EXPORTS
{
    entity Template = {};
    Template.Type = ENTITY_PLAYER;
    Template.Color = VA_MAROON;
    Template.Glyph = '@';
    Template.Condition = 100.0f;
    Template.ActionCost = 100;
    Template.ViewRange = 30;
    Template.Name = "Player";
    Template.Description = "After Derval's disappearance you awake in the Souterrain. You've read about this place in the dusty tomes kept in St Catherine's Library. You remember one thing: the only way is down.";
    SetFlags(&Template.Flags, ENTITY_IS_BLOCKING);

    Template.Health = Template.MaxHealth = 10;
    Template.ArmorClass = 10;
    Template.AttackModifier = 10;
    Template.Damage = 3;

    Template.RegenActionCost = 500;
    Template.RegenAmount = 2;

    return Template;
}
#else
;
#endif

TEMPLATE_FUNC entity
Template_AetherFly()
#ifdef TEMPLATE_EXPORTS
{
    entity Template = {};
    Template.Type = ENTITY_NPC;
    Template.Color = VA_CORAL;
    Template.Glyph = 1 + 9*16;
    Template.Condition = 100.0f;
    Template.ActionCost = 80;
    Template.ViewRange = 15;
    Template.NpcState = NPC_STATE_IDLE;
    Template.Name = "Aether Fly";
    Template.Description = "Sentient dipteron from the outer realms. Condemned to roam the Souterrain for eternity by the jealous goddess Latena.";
    SetFlags(&Template.Flags, ENTITY_IS_BLOCKING);

    Template.Health = Template.MaxHealth = 3;
    Template.ArmorClass = 13;
    Template.AttackModifier = 3;
    Template.Damage = 1;
    
    return Template;
}
#else
;
#endif

// TODO: Aether ant
// TODO: Martyr worshipped by aether creatures (boss)
// TODO: Trees that are seemingly inanimate, but wake up after a delay
// TODO: Derval - main boss; Derval's ring - goal
// TODO: A vagaband - maybe boss? or friendly npc???????????????????????
