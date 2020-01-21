/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

/* ScriptData
Name: cast_commandscript
%Complete: 100
Comment: All cast related commands
Category: commandscripts
EndScriptData */

#include "ScriptMgr.h"
#include "Chat.h"
#include "Creature.h"
#include "Language.h"
#include "Player.h"
#include "RBAC.h"
#include "SpellMgr.h"
#include "WorldSession.h"
#include "ObjectMgr.h"

class cast_commandscript : public CommandScript
{
public:
    cast_commandscript() : CommandScript("cast_commandscript") { }

    std::vector<ChatCommand> GetCommands() const override
    {
        static std::vector<ChatCommand> castCommandTable =
        {
            { "back",   rbac::RBAC_PERM_COMMAND_CAST_BACK,   false, &HandleCastBackCommand,  "" },
            { "dist",   rbac::RBAC_PERM_COMMAND_CAST_DIST,   false, &HandleCastDistCommand,  "" },
            { "self",   rbac::RBAC_PERM_COMMAND_CAST_SELF,   false, &HandleCastSelfCommand,  "" },
            { "target", rbac::RBAC_PERM_COMMAND_CAST_TARGET, false, &HandleCastTargetCommand, "" },
            { "dest",   rbac::RBAC_PERM_COMMAND_CAST_DEST,   false, &HandleCastDestCommand,  "" },
            { "",       rbac::RBAC_PERM_COMMAND_CAST,        false, &HandleCastCommand,      "" },
        };
        static std::vector<ChatCommand> commandTable =
        {
            { "cast",   rbac::RBAC_PERM_COMMAND_CAST,        false, NULL,                    "", castCommandTable },
        };
        return commandTable;
    }

    static bool CheckSpellExistsAndIsValid(ChatHandler* handler, uint32 spellId)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo)
        {
            handler->PSendSysMessage(LANG_COMMAND_NOSPELLFOUND);
            handler->SetSentErrorMessage(true);
            return false;
        }

        if (!SpellMgr::IsSpellValid(spellInfo, handler->GetSession()->GetPlayer()))
        {
            handler->PSendSysMessage(LANG_COMMAND_SPELL_BROKEN, spellId);
            handler->SetSentErrorMessage(true);
            return false;
        }
        return true;
    }

    static bool HandleCastCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Unit* target = handler->getSelectedUnit();
        if (!target)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        if (!CheckSpellExistsAndIsValid(handler, spellId))
            return false;

        char* triggeredStr = strtok(NULL, " ");
        if (triggeredStr)
        {
            int l = strlen(triggeredStr);
            if (strncmp(triggeredStr, "triggered", l) != 0)
                return false;
        }

        TriggerCastFlags triggered = (triggeredStr != NULL) ? TRIGGERED_FULL_DEBUG_MASK : TRIGGERED_NONE;
        handler->GetSession()->GetPlayer()->CastSpell(target, spellId, triggered);

        return true;
    }

    static bool HandleCastBackCommand(ChatHandler* handler, char const* args)
    {
        Creature* caster = handler->getSelectedCreature();
        if (!caster)
        {
            handler->SendSysMessage(LANG_SELECT_CHAR_OR_CREATURE);
            handler->SetSentErrorMessage(true);
            return false;
        }

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        if (!CheckSpellExistsAndIsValid(handler, spellId))
            return false;

        char* triggeredStr = strtok(NULL, " ");
        if (triggeredStr)
        {
            int l = strlen(triggeredStr);
            if (strncmp(triggeredStr, "triggered", l) != 0)
                return false;
        }

        TriggerCastFlags triggered = (triggeredStr != NULL) ? TRIGGERED_FULL_DEBUG_MASK : TRIGGERED_NONE;
        caster->CastSpell(handler->GetSession()->GetPlayer(), spellId, triggered);

        return true;
    }

    static bool HandleCastDistCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        if (!CheckSpellExistsAndIsValid(handler, spellId))
            return false;

        char* distStr = strtok(NULL, " ");

        float dist = 0;

        if (distStr)
            sscanf(distStr, "%f", &dist);

        char* triggeredStr = strtok(NULL, " ");
        if (triggeredStr)
        {
            int l = strlen(triggeredStr);
            if (strncmp(triggeredStr, "triggered", l) != 0)
                return false;
        }

        TriggerCastFlags triggered = (triggeredStr != NULL) ? TRIGGERED_FULL_DEBUG_MASK : TRIGGERED_NONE;
        float x, y, z;
        handler->GetSession()->GetPlayer()->GetClosePoint(x, y, z, dist);

        handler->GetSession()->GetPlayer()->CastSpell(x, y, z, spellId);

        return true;
    }

    static bool HandleCastSelfCommand(ChatHandler* handler, char const* args)
    {
        if (!*args)
            return false;

        Unit* target = handler->getSelectedUnit();

        // number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
        uint32 spellId = handler->extractSpellIdFromLink((char*)args);
        if (!spellId)
            return false;

        if (!CheckSpellExistsAndIsValid(handler, spellId))
            return false;

        target->CastSpell(target, spellId, false);

        return true;
    }

static bool HandleCastTargetCommand(ChatHandler* handler, char const* args)
{
 if (!*args)
        return false;

 Player* player = handler->GetSession()->GetPlayer();
 Player* target = player->GetSelectedPlayer();

    if (!target) {
        handler->SendSysMessage("Error a la hora de seleccionar objetivo.");
    }

    char* arg1 = strtok((char*)args, " ");

    uint32 lowguid = atoull(arg1);
    if (!lowguid)
        return false;

    Creature* creature = handler->GetCreatureFromPlayerMapByDbGuid(lowguid);
	if (!creature)
		return false;

    char* arg2 = strtok(NULL, "]");
	

    if (!arg1 || !arg2)
    {
        //Se va a la verga
        handler->SendSysMessage("Uso incorrecto del comando. Estructura a emplear: .cast target GUID spellID");
        return false;
    }
    else {
		uint32 spellId = handler->extractSpellIdFromLink((char*)args);
		if (!spellId)
			return false;
	if (!spellId)
		return false;
		
         creature->CastSpell(target, spellId, false);
        handler->SendSysMessage("Spell spawneada con éxito");

        return true;
}
}

static bool HandleCastDestCommand(ChatHandler* handler, char const* args)
{
	if (!*args)
		return false;

	Unit* caster = handler->GetSession()->GetPlayer();

	// number or [name] Shift-click form |color|Hspell:spell_id|h[name]|h|r or Htalent form
	uint32 spellId = handler->extractSpellIdFromLink((char*)args);
	if (!spellId)
		return false;

	if (!CheckSpellExistsAndIsValid(handler, spellId))
		return false;

	char* arg1 = strtok(NULL, " ");
	char* arg2 = strtok(NULL, " ");
	
	float x, y, z, o;
	x = caster->GetPositionX();
	y = caster->GetPositionY();
	z = caster->GetPositionZ();
	o = caster->GetOrientation();
	if (!arg1)
	{
		caster->CastSpell(x, y, z, spellId);
		return true;
	} else {
		
			do {
				char* dir = arg1;
				float value = float(atof(arg2));

				switch (dir[0])
				{
				case 'l':
					x = x + value;
					//y = y + sin(o + (M_PI / 2))*value;
					break;
				case 'r':
					x = x + cos(o - (M_PI / 2))*value;
					y = y + sin(o - (M_PI / 2))*value;
					break;
				case 'f':
					x = x + cosf(o)*value;
					y = y + sinf(o)*value;
					break;
				case 'b':
					x = x - cosf(o)*value;
					y = y - sinf(o)*value;
					break;
				case 'u':
					z = z + value;
					break;
				case 'd':
					z = z - value;
					break;
				default:
					handler->PSendSysMessage("Uso incorrecto del comando");
					return false;
				}
				arg1 = strtok(NULL, " ");
				arg2 = strtok(NULL, " ");
			} while ((arg1 != NULL && arg2 != NULL));

			Map* map = caster->GetMap();
			uint32 id = 90493;
			Creature* creature = Creature::CreateCreature(id, map, caster->GetPosition());
			creature->Relocate(x, y, z);
			creature->SaveToDB(map->GetId(), { map->GetDifficultyID() });
			ObjectGuid::LowType db_guid = creature->GetSpawnId();
			creature->CleanupsBeforeDelete();
			delete creature;
			creature = Creature::CreateCreatureFromDB(db_guid, map);
			sObjectMgr->AddCreatureToGrid(db_guid, sObjectMgr->GetCreatureData(db_guid));

			creature->CastSpell(creature, spellId, false);

			creature->DeleteFromDB();
			creature->AddObjectToRemoveList();
			handler->SendSysMessage("Spell spawneada con éxito");
			return true;
	}
		}
};

void AddSC_cast_commandscript()
{
    new cast_commandscript();
}
