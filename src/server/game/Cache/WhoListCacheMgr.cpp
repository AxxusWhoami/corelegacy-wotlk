/*
 * This file is part of the AzerothCore Project. See AUTHORS file for Copyright information
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "WhoListCacheMgr.h"
#include "AreaDefines.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "World.h"
#include "WorldSessionMgr.h"

WhoListCacheMgr* WhoListCacheMgr::instance()
{
    static WhoListCacheMgr instance;
    return &instance;
}

void WhoListCacheMgr::Update()
{
    // clear current list
    _whoListStorage.clear();
    _whoListStorage.reserve(sWorldSessionMgr->GetPlayerCount() + 1);

    for (auto const& [guid, player] : ObjectAccessor::GetPlayers())
    {
        // Blindaje Nivel 1: Validar integridad del objeto Player y su presencia real en el mundo
        if (!player || !player->IsInWorld() || !player->FindMap())
            continue;

        // Blindaje Nivel 2: Extracción segura del puntero de sesión
        WorldSession* session = player->GetSession();
        if (!session)
            continue;

        // Blindaje Nivel 3: Mitigación específica para memory leaks de mod-playerbots
        // Descomenta la siguiente línea si los bots están causando el crash al dejar sesiones colgadas
        // if (player->IsBot()) continue; 

        // Validación original ahora protegida contra accesos prematuros
        if (session->PlayerLoading())
            continue;

        std::string playerName = player->GetName();
        std::wstring widePlayerName;

        if (!Utf8toWStr(playerName, widePlayerName))
            continue;

        wstrToLower(widePlayerName);

        std::string guildName = sGuildMgr->GetGuildNameById(player->GetGuildId());
        std::wstring wideGuildName;

        if (!Utf8toWStr(guildName, wideGuildName))
            continue;

        wstrToLower(wideGuildName);

        // Se reutiliza la variable 'session' validada previamente en lugar de encadenar player->GetSession() nuevamente
        _whoListStorage.emplace_back(player->GetGUID(), player->GetTeamId(), session->GetSecurity(), player->GetLevel(),
            player->getClass(), player->getRace(),
            (player->IsSpectator() ? AREA_DALARAN : player->GetZoneId()), player->getGender(), player->IsVisible(),
            widePlayerName, wideGuildName, playerName, guildName);
    }
}
