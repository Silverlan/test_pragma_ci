/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Copyright (c) 2020 Florian Weischer
 */

#ifndef __C_SHADER_PREPASS_HPP__
#define __C_SHADER_PREPASS_HPP__

#include "pragma/clientdefinitions.h"
#include "pragma/rendering/shaders/world/c_shader_scene.hpp"
#include <shader/prosper_shader.hpp>

namespace pragma
{
	class DLLCLIENT ShaderPrepassBase
		: public ShaderGameWorld
	{
	public:
		enum class Pipeline : uint32_t
		{
			Regular = umath::to_integral(ShaderEntity::Pipeline::Regular),
			MultiSample = umath::to_integral(ShaderEntity::Pipeline::MultiSample),

			Reflection = umath::to_integral(ShaderEntity::Pipeline::Count),
			Count
		};
		enum class Flags : uint32_t
		{
			None = 0u,
			UseExtendedVertexWeights = 1u,
			RenderAs3DSky = UseExtendedVertexWeights<<1u,
			AlphaTest = RenderAs3DSky<<1u
		};
		enum class MaterialBinding : uint32_t
		{
			AlbedoMap = 0u,

			Count
		};
		static Pipeline GetPipelineIndex(prosper::SampleCountFlags sampleCount);
		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_BONE_WEIGHT_EXT;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT_ID;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_BONE_WEIGHT_EXT;

		static prosper::ShaderGraphics::VertexBinding VERTEX_BINDING_VERTEX;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_POSITION;
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_UV;

		static prosper::DescriptorSetInfo DESCRIPTOR_SET_INSTANCE;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_CAMERA;
		static prosper::DescriptorSetInfo DESCRIPTOR_SET_MATERIAL;

#pragma pack(push,1)
		struct PushConstants
		{
			Vector4 clipPlane;
			Vector4 drawOrigin; // w is scale
			uint32_t vertexAnimInfo;
			Flags flags;
			float alphaCutoff;
		};
#pragma pack(pop)

		ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier,const std::string &vsShader,const std::string &fsShader);
		ShaderPrepassBase(prosper::IPrContext &context,const std::string &identifier);

		virtual bool BeginDraw(
			const std::shared_ptr<prosper::IPrimaryCommandBuffer> &cmdBuffer,const Vector4 &clipPlane,const Vector4 &drawOrigin={0.f,0.f,0.f,1.f},ShaderGameWorldPipeline pipelineIdx=ShaderGameWorldPipeline::Regular,
			RecordFlags recordFlags=RecordFlags::RenderPassTargetAsViewportAndScissor
		) override;
		virtual bool BindClipPlane(const Vector4 &clipPlane) override;
		virtual bool BindScene(pragma::CSceneComponent &scene,rendering::RasterizationRenderer &renderer,bool bView) override;
		virtual bool BindDrawOrigin(const Vector4 &drawOrigin) override;
		virtual void Set3DSky(bool is3dSky) override;
		virtual bool Draw(CModelSubMesh &mesh,const std::optional<pragma::RenderMeshIndex> &meshIdx) override;
	protected:
		virtual bool BindMaterial(CMaterial &mat) override;
		virtual std::shared_ptr<prosper::IDescriptorSetGroup> InitializeMaterialDescriptorSet(CMaterial &mat) override;
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
		
		uint32_t GetMaterialDescriptorSetIndex() const;
		virtual uint32_t GetCameraDescriptorSetIndex() const override;
		virtual uint32_t GetInstanceDescriptorSetIndex() const override;
		virtual void GetVertexAnimationPushConstantInfo(uint32_t &offset) const override;
	private:
		// These are unused
		virtual uint32_t GetRenderSettingsDescriptorSetIndex() const override {return std::numeric_limits<uint32_t>::max();}
		virtual uint32_t GetLightDescriptorSetIndex() const {return std::numeric_limits<uint32_t>::max();}
		virtual bool BindRenderSettings(prosper::IDescriptorSet &descSetRenderSettings) override {return false;}
		virtual bool BindLights(prosper::IDescriptorSet &dsLights) override {return false;}
		Flags m_stateFlags = Flags::None;
		std::optional<float> m_alphaCutoff {};
	};
	using ShaderPrepassDepth = ShaderPrepassBase;

	//////////////////

	class DLLCLIENT ShaderPrepass
		: public ShaderPrepassBase
	{
	public:
		static prosper::ShaderGraphics::VertexAttribute VERTEX_ATTRIBUTE_NORMAL;

		static prosper::Format RENDER_PASS_NORMAL_FORMAT;

		ShaderPrepass(prosper::IPrContext &context,const std::string &identifier);
	protected:
		virtual void InitializeRenderPass(std::shared_ptr<prosper::IRenderPass> &outRenderPass,uint32_t pipelineIdx) override;
		virtual void InitializeGfxPipeline(prosper::GraphicsPipelineCreateInfo &pipelineInfo,uint32_t pipelineIdx) override;
	};
};
REGISTER_BASIC_BITWISE_OPERATORS(pragma::ShaderPrepassBase::Flags)

#endif