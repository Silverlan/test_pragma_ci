/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#include "stdafx_client.h"
#include "pragma/entities/components/c_light_map_receiver_component.hpp"
#include "pragma/model/c_model.h"
#include "pragma/model/c_modelmesh.h"
#include "pragma/model/vk_mesh.h"
#include <pragma/entities/entity_component_system_t.hpp>

extern DLLCLIENT CGame *c_game;
extern DLLCLIENT ClientState *client;
extern DLLCENGINE CEngine *c_engine;

using namespace pragma;

void CLightMapReceiverComponent::SetupLightMapUvData(CBaseEntity &ent)
{
	auto mdl = ent.GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0u) : nullptr;
	if(meshGroup == nullptr)
		return;
	auto hasLightmapUvs = false;
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto *uvSet = subMesh->GetUVSet("lightmap");
			if(uvSet)
			{
				hasLightmapUvs = true;
				goto endLoop;
			}
		}
	}
endLoop:
	if(hasLightmapUvs)
	{
		auto lightMapReceiverC = ent.AddComponent<CLightMapReceiverComponent>();
		if(lightMapReceiverC.valid())
			lightMapReceiverC->UpdateLightMapUvData();
	}
}
void CLightMapReceiverComponent::UpdateLightMapUvData()
{
	auto mdl = GetEntity().GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0u) : nullptr;
	if(meshGroup == nullptr)
		return;
	m_modelName = GetEntity().GetModelName();
	umath::set_flag(m_stateFlags,StateFlags::IsModelBakedWithLightMaps,true);
	m_uvDataPerMesh.clear();
	m_meshes.clear();
	m_meshToMeshIdx.clear();
	m_meshToBufIdx.clear();
	umath::set_flag(m_stateFlags,StateFlags::RenderMeshBufferIndexTableDirty);
	uint32_t subMeshIdx = 0u;
	auto wasInitialized = false;
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto *uvSet = subMesh->GetUVSet("lightmap");
			if(uvSet == nullptr)
			{
				++subMeshIdx;
				continue;
			}
			m_uvDataPerMesh.insert(std::make_pair(subMeshIdx,*uvSet));
			m_meshes.insert(std::make_pair(subMeshIdx,subMesh));
			m_meshToMeshIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh.get()),subMeshIdx));
			++subMeshIdx;
		}
	}
	UpdateRenderMeshBufferList();
}
luabind::object CLightMapReceiverComponent::InitializeLuaObject(lua_State *l) {return BaseEntityComponent::InitializeLuaObject<CLightMapReceiverComponentHandleWrapper>(l);}
void CLightMapReceiverComponent::Initialize()
{
	BaseEntityComponent::Initialize();
	BindEventUnhandled(CModelComponent::EVENT_ON_MODEL_CHANGED,[this](std::reference_wrapper<ComponentEvent> evData) {
		//m_isModelBakedWithLightMaps = (GetEntity().GetModelName() == m_modelName); // TODO
		//if(m_isModelBakedWithLightMaps)
		m_meshBufferIndices.clear();
		UpdateModelMeshes();
	});
	BindEventUnhandled(CModelComponent::EVENT_ON_RENDER_MESHES_UPDATED,[this](std::reference_wrapper<ComponentEvent> evData) {
		UpdateRenderMeshBufferList();
	});
	BindEventUnhandled(CBaseEntity::EVENT_ON_SPAWN,[this](std::reference_wrapper<ComponentEvent> evData) {
		//m_isModelBakedWithLightMaps = (GetEntity().GetModelName() == m_modelName); // TODO
		//if(m_isModelBakedWithLightMaps)
			UpdateModelMeshes();
	});
	if(GetEntity().IsSpawned())
	{
		UpdateModelMeshes();
		UpdateRenderMeshBufferList();
	}
}
void CLightMapReceiverComponent::UpdateRenderMeshBufferList()
{
	m_meshBufferIndices.clear();
	auto mdlC = GetEntity().GetModelComponent();
	if(mdlC.expired())
		return;
	umath::set_flag(m_stateFlags,StateFlags::RenderMeshBufferIndexTableDirty,false);
	auto &renderMeshes = static_cast<CModelComponent*>(mdlC.get())->GetRenderMeshes();
	m_meshBufferIndices.resize(renderMeshes.size());
	for(auto i=decltype(renderMeshes.size()){0u};i<renderMeshes.size();++i)
	{
		auto bufIdx = FindBufferIndex(static_cast<CModelSubMesh&>(*renderMeshes[i]));
		m_meshBufferIndices[i] = bufIdx.has_value() ? *bufIdx : std::numeric_limits<BufferIdx>::max();
	}
}
void CLightMapReceiverComponent::UpdateModelMeshes()
{
	auto mdl = GetEntity().GetModel();
	if(mdl == nullptr)
		return;
	m_meshes.clear();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0u) : nullptr;
	if(meshGroup == nullptr)
		return;
	std::unordered_map<MeshIdx,BufferIdx> meshIdxToBufIdx {};
	for(auto &pair : m_meshToMeshIdx)
	{
		auto *mesh = pair.first;
		auto it = m_meshToBufIdx.find(mesh);
		if(it == m_meshToBufIdx.end())
			continue;
		meshIdxToBufIdx.insert(std::make_pair(pair.second,it->second));
	}
	umath::set_flag(m_stateFlags,StateFlags::RenderMeshBufferIndexTableDirty);
	m_meshToBufIdx.clear();
	m_meshToMeshIdx.clear();
	uint32_t subMeshIdx = 0u;
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto *uvSet = subMesh->GetUVSet("lightmap");
			if(uvSet == nullptr)
			{
				++subMeshIdx;
				continue;
			}
			m_meshes.insert(std::make_pair(subMeshIdx,subMesh));
			auto itBufIdx = meshIdxToBufIdx.find(subMeshIdx);
			if(itBufIdx != meshIdxToBufIdx.end())
			{
				m_meshToBufIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh.get()),itBufIdx->second));
				m_meshToMeshIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(subMesh.get()),subMeshIdx));
			}
			++subMeshIdx;
		}
	}
}
const std::unordered_map<CLightMapReceiverComponent::MeshIdx,std::vector<Vector2>> &CLightMapReceiverComponent::GetMeshLightMapUvData() const {return m_uvDataPerMesh;}
void CLightMapReceiverComponent::AssignBufferIndex(MeshIdx meshIdx,BufferIdx bufIdx)
{
	auto itMesh = m_meshes.find(meshIdx);
	if(itMesh == m_meshes.end())
		return;
	m_meshToBufIdx.insert(std::make_pair(static_cast<CModelSubMesh*>(itMesh->second.get()),bufIdx));
	umath::set_flag(m_stateFlags,StateFlags::RenderMeshBufferIndexTableDirty);
}
std::optional<pragma::CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::GetBufferIndex(RenderMeshIndex meshIdx) const
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::RenderMeshBufferIndexTableDirty))
		const_cast<CLightMapReceiverComponent*>(this)->UpdateRenderMeshBufferList();
	return (meshIdx < m_meshBufferIndices.size() && m_meshBufferIndices[meshIdx] != std::numeric_limits<BufferIdx>::max()) ? m_meshBufferIndices[meshIdx] : std::optional<pragma::CLightMapReceiverComponent::BufferIdx>{};
}
std::optional<CLightMapReceiverComponent::BufferIdx> CLightMapReceiverComponent::FindBufferIndex(CModelSubMesh &mesh) const
{
	if(umath::is_flag_set(m_stateFlags,StateFlags::IsModelBakedWithLightMaps) == false)
		return {};
	auto it = m_meshToBufIdx.find(&mesh);
	if(it == m_meshToBufIdx.end())
		return {};
	return it->second;
}
void CLightMapReceiverComponent::UpdateMeshLightmapUvBuffers(CLightMapComponent &lightMapC)
{
	auto mdl = GetEntity().GetModel();
	auto meshGroup = mdl ? mdl->GetMeshGroup(0u) : nullptr;
	if(meshGroup == nullptr)
		return;
	for(auto &mesh : meshGroup->GetMeshes())
	{
		for(auto &subMesh : mesh->GetSubMeshes())
		{
			auto &sceneMesh = static_cast<CModelSubMesh*>(subMesh.get())->GetSceneMesh();
			auto bufIdx = FindBufferIndex(*static_cast<CModelSubMesh*>(subMesh.get()));
			if(bufIdx.has_value() == false || sceneMesh == nullptr)
				continue;
			auto *pUvBuffer = lightMapC.GetMeshLightMapUvBuffer(*bufIdx);
			prosper::IBuffer *pLightMapUvBuffer = nullptr;
			if(pUvBuffer != nullptr)
				pLightMapUvBuffer = pUvBuffer;
			else
				pLightMapUvBuffer = c_engine->GetRenderContext().GetDummyBuffer().get();
			sceneMesh->SetLightmapUvBuffer(pLightMapUvBuffer->shared_from_this());
		}
	}
}
