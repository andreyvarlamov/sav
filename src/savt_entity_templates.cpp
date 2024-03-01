entity
Template_PumiceWall()
{
    entity Template = {};
    Template.Type = ENTITY_STATIC;
    Template.Color = VA_SLATEGRAY;
    Template.Glyph = 11 + 16*13;
    Template.Health = Template.MaxHealth = 100.0f;
    Template.Name = "Pumice Wall";
    Template.Description = "Wall made of soft pumice rock.";
    SetFlags(&Template.Flags, ENTITY_IS_BLOCKING | ENTITY_IS_OPAQUE);
    return Template;
}

entity
Template_Player()
{
    entity Template = {};
    Template.Type = ENTITY_PLAYER;
    Template.Color = VA_MAROON;
    Template.Glyph = '@';
    Template.Health = Template.MaxHealth = 100.0f;
    Template.ActionCost = 100;
    Template.ViewRange = 100;
    Template.Name = "Player";
    Template.Description = "After Derval's disappearance you awake in the Souterrain. You've read about this place in the dusty tomes kept in St Catherine's Library. You remember one thing: the only way is down.";
    SetFlags(&Template.Flags, ENTITY_IS_BLOCKING);
    return Template;
}

entity
Template_AetherFly()
{
    entity Template = {};
    Template.Type = ENTITY_NPC;
    Template.Color = VA_CORAL;
    Template.Glyph = 1 + 9*16;
    Template.Health = Template.MaxHealth = 10.0f;
    Template.ActionCost = 110;
    Template.ViewRange = 10;
    Template.NpcState = NPC_STATE_IDLE;
    Template.Name = "Aether Fly";
    Template.Description = "Sentient dipteron from the outer realms. Condemned to roam the Souterrain for eternity by the jealous goddess Latena.";
    SetFlags(&Template.Flags, ENTITY_IS_BLOCKING);
    return Template;
}

// TODO: Aether ant
// TODO: Martyr worshipped by aether creatures (boss)
// TODO: Trees that are seemingly inanimate, but wake up after a delay
// TODO: Derval - main boss; Derval's ring - goal
// TODO: A vagaband - maybe boss? or friendly npc???????????????????????
