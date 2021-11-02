/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan
 */

#include "stdafx_client.h"
#include "pragma/entities/point/c_point_target.h"
#include "pragma/entities/components/c_transform_component.hpp"
#include "pragma/entities/c_entityfactories.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include <pragma/lua/converters/game_type_converters_t.hpp>
#include <pragma/entities/entity_component_system_t.hpp>

using namespace pragma;

LINK_ENTITY_TO_CLASS(point_target,CPointTarget);

void CPointTargetComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<std::remove_reference_t<decltype(*this)>>(l);}
void CPointTarget::Initialize()
{
	CBaseEntity::Initialize();
	AddComponent<CTransformComponent>();
	AddComponent<CPointTargetComponent>();
}
