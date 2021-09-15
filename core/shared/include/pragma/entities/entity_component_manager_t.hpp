/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2021 Silverlan */

#ifndef __ENTITY_COMPONENT_MANAGER_T_HPP__
#define __ENTITY_COMPONENT_MANAGER_T_HPP__

#include "pragma/entities/entity_component_manager.hpp"
#include "pragma/entities/attribute_specialization_type.hpp"
#include <sharedutils/util.h>
#include <udm.hpp>

class BaseEntity;
namespace pragma
{
	constexpr bool is_animatable_type(udm::Type type)
	{
		return !udm::is_non_trivial_type(type) && type != udm::Type::HdrColor && type != udm::Type::Srgba &&
			type != udm::Type::Transform && type != udm::Type::ScaledTransform && type != udm::Type::Nil &&
			type != udm::Type::Half;
	}
	template<typename T>
		concept is_animatable_type_v = is_animatable_type(udm::type_to_enum<T>());

	template<typename TComponent,typename T,auto TSetter,auto TGetter,typename TSpecializationType> requires(is_animatable_type_v<T> && (std::is_same_v<TSpecializationType,AttributeSpecializationType> || util::is_string<TSpecializationType>::value))
		static ComponentMemberInfo create_component_member_info(std::string &&name,std::optional<T> defaultValue,TSpecializationType specialization)
	{
		auto memberInfo = ComponentMemberInfo::CreateDummy();
		memberInfo.SetName(std::move(name));
		memberInfo.type = udm::type_to_enum<T>();
		memberInfo.SetGetterFunction<TComponent,T,TGetter>();
		memberInfo.SetSetterFunction<TComponent,T,TSetter>();
		memberInfo.SetSpecializationType(specialization);
		if(defaultValue.has_value())
			memberInfo.SetDefault<T>(*defaultValue);
		return memberInfo;
	}

	template<typename TComponent,typename T,auto TSetter,auto TGetter> requires(is_animatable_type_v<T>)
		static ComponentMemberInfo create_component_member_info(std::string &&name,std::optional<T> defaultValue={})
	{
		return create_component_member_info<TComponent,T,TSetter,TGetter,AttributeSpecializationType>(std::move(name),std::move(defaultValue),AttributeSpecializationType::None);
	}

	template<typename T>
		void ComponentMemberInfo::SetDefault(T value)
	{
		if(udm::type_to_enum<T>() != type)
			throw std::runtime_error{"Unable to set default member value: Value type " +std::string{magic_enum::enum_name(udm::type_to_enum<T>())} +" does not match member type " +std::string{magic_enum::enum_name(type)} +"!"};
		m_default = std::unique_ptr<void,void(*)(void*)>{new T{std::move(value)},[](void *ptr) {
			delete static_cast<T*>(ptr);
		}};
	}
};

#endif
