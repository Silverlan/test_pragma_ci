#include "stdafx_client.h"
#include "pragma/game/c_game.h"
#include "pragma/lua/c_lentity_handles.hpp"
#include "pragma/lua/classes/components/c_lentity_components.hpp"
#include "pragma/lua/classes/lproperty.hpp"
#include "pragma/model/c_modelmesh.h"
#include "pragma/rendering/renderers/rasterization_renderer.hpp"
#include <pragma/lua/classes/lproperty_generic.hpp>
#include <pragma/lua/classes/ldef_vector.h>
#include <pragma/lua/classes/ldef_color.h>
#include <pragma/lua/classes/ldef_angle.h>
#include <pragma/lua/classes/ldef_quaternion.h>
#include <pragma/model/model.h>
#include <pragma/physics/raytraces.h>
#include <pragma/lua/lentity_components_base_types.hpp>
#include <pragma/lua/lentity_components.hpp>
#include <pragma/lua/lua_entity_component.hpp>
#include <prosper_command_buffer.hpp>
#include <prosper_descriptor_set_group.hpp>
#include <pragma/physics/movetypes.h>
#include <pragma/lua/lua_call.hpp>

namespace Lua
{
	namespace ParticleSystem
	{
		static std::string get_key_value(lua_State *l,int32_t argIdx)
		{
			if(Lua::IsNumber(l,argIdx))
				return std::to_string(Lua::CheckNumber(l,argIdx));
			if(Lua::IsBool(l,argIdx))
				return Lua::CheckBool(l,argIdx) ? "1" : "0";
			if(Lua::IsVector4(l,argIdx))
			{
				auto &v = *Lua::CheckVector4(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z) +" " +std::to_string(v.w);
			}
			if(Lua::IsVector(l,argIdx))
			{
				auto &v = *Lua::CheckVector(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z);
			}
			if(Lua::IsVector2(l,argIdx))
			{
				auto &v = *Lua::CheckVector2(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y);
			}
			if(Lua::IsVector4i(l,argIdx))
			{
				auto &v = *Lua::CheckVector4i(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z) +" " +std::to_string(v.w);
			}
			if(Lua::IsVectori(l,argIdx))
			{
				auto &v = *Lua::CheckVectori(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y) +" " +std::to_string(v.z);
			}
			if(Lua::IsVector2i(l,argIdx))
			{
				auto &v = *Lua::CheckVector2i(l,argIdx);
				return std::to_string(v.x) +" " +std::to_string(v.y);
			}
			if(Lua::IsColor(l,argIdx))
			{
				auto &c = *Lua::CheckColor(l,argIdx);
				return std::to_string(c.r) +" " +std::to_string(c.g) +" " +std::to_string(c.b) +" " +std::to_string(c.a);
			}
			if(Lua::IsEulerAngles(l,argIdx))
			{
				auto &ang = *Lua::CheckEulerAngles(l,argIdx);
				return std::to_string(ang.p) +" " +std::to_string(ang.y) +" " +std::to_string(ang.r);
			}
			if(Lua::IsQuaternion(l,argIdx))
			{
				auto &rot = *Lua::CheckQuaternion(l,argIdx);
				return std::to_string(rot.w) +" " +std::to_string(rot.x) +" " +std::to_string(rot.y) +" " +std::to_string(rot.z);
			}
			return Lua::CheckString(l,argIdx);
		}
	};
	namespace Decal
	{
		static void create_from_projection(lua_State *l,CDecalHandle &hComponent,luabind::object tMeshes,const pragma::physics::ScaledTransform &pose)
		{
			pragma::Lua::check_component(l,hComponent);
			int32_t t = 2;
			Lua::CheckTable(l,t);
			std::vector<ModelSubMesh*> meshes {};
			auto numMeshes = Lua::GetObjectLength(l,t);
			for(auto i=decltype(numMeshes){0u};i<numMeshes;++i)
			{
				Lua::PushInt(l,i +1);
				Lua::GetTableValue(l,t);
				auto &mesh = Lua::Check<ModelSubMesh>(l,-1);
				meshes.push_back(&mesh);
				Lua::Pop(l,1);
			}
			Lua::PushBool(l,hComponent->ApplyDecal(meshes,pose));
		}
		static void create_from_projection(lua_State *l,CDecalHandle &hComponent,luabind::object tMeshes)
		{
			create_from_projection(l,hComponent,tMeshes,{});
		}
	};
};

void CGame::RegisterLuaEntityComponents(luabind::module_ &entsMod)
{
	Game::RegisterLuaEntityComponents(entsMod);
	auto *l = GetLuaState();
	Lua::register_cl_ai_component(l,entsMod);
	Lua::register_cl_character_component(l,entsMod);
	Lua::register_cl_player_component(l,entsMod);
	Lua::register_cl_vehicle_component(l,entsMod);
	Lua::register_cl_weapon_component(l,entsMod);

	auto defCColor = luabind::class_<CColorHandle,BaseEntityComponentHandle>("ColorComponent");
	Lua::register_base_color_component_methods<luabind::class_<CColorHandle,BaseEntityComponentHandle>,CColorHandle>(l,defCColor);
	entsMod[defCColor];

	auto defCScore = luabind::class_<CScoreHandle,BaseEntityComponentHandle>("ScoreComponent");
	Lua::register_base_score_component_methods<luabind::class_<CScoreHandle,BaseEntityComponentHandle>,CScoreHandle>(l,defCScore);
	entsMod[defCScore];

	auto defCFlammable = luabind::class_<CFlammableHandle,BaseEntityComponentHandle>("FlammableComponent");
	Lua::register_base_flammable_component_methods<luabind::class_<CFlammableHandle,BaseEntityComponentHandle>,CFlammableHandle>(l,defCFlammable);
	entsMod[defCFlammable];

	auto defCHealth = luabind::class_<CHealthHandle,BaseEntityComponentHandle>("HealthComponent");
	Lua::register_base_health_component_methods<luabind::class_<CHealthHandle,BaseEntityComponentHandle>,CHealthHandle>(l,defCHealth);
	entsMod[defCHealth];

	auto defCName = luabind::class_<CNameHandle,BaseEntityComponentHandle>("NameComponent");
	Lua::register_base_name_component_methods<luabind::class_<CNameHandle,BaseEntityComponentHandle>,CNameHandle>(l,defCName);
	entsMod[defCName];

	auto defCNetworked = luabind::class_<CNetworkedHandle,BaseEntityComponentHandle>("NetworkedComponent");
	Lua::register_base_networked_component_methods<luabind::class_<CNetworkedHandle,BaseEntityComponentHandle>,CNetworkedHandle>(l,defCNetworked);
	entsMod[defCNetworked];

	auto defCObservable = luabind::class_<CObservableHandle,BaseEntityComponentHandle>("ObservableComponent");
	Lua::register_base_observable_component_methods<luabind::class_<CObservableHandle,BaseEntityComponentHandle>,CObservableHandle>(l,defCObservable);
	entsMod[defCObservable];

	auto defCShooter = luabind::class_<CShooterHandle,BaseEntityComponentHandle>("ShooterComponent");
	Lua::register_base_shooter_component_methods<luabind::class_<CShooterHandle,BaseEntityComponentHandle>,CShooterHandle>(l,defCShooter);
	entsMod[defCShooter];

	auto defCPhysics = luabind::class_<CPhysicsHandle,BaseEntityComponentHandle>("PhysicsComponent");
	Lua::register_base_physics_component_methods<luabind::class_<CPhysicsHandle,BaseEntityComponentHandle>,CPhysicsHandle>(l,defCPhysics);
	entsMod[defCPhysics];

	auto defCRadius = luabind::class_<CRadiusHandle,BaseEntityComponentHandle>("RadiusComponent");
	Lua::register_base_radius_component_methods<luabind::class_<CRadiusHandle,BaseEntityComponentHandle>,CRadiusHandle>(l,defCRadius);
	entsMod[defCRadius];

	auto defCWorld = luabind::class_<CWorldHandle,BaseEntityComponentHandle>("WorldComponent");
	Lua::register_base_world_component_methods<luabind::class_<CWorldHandle,BaseEntityComponentHandle>,CWorldHandle>(l,defCWorld);
	defCWorld.def("GetBSPTree",static_cast<void(*)(lua_State*,CWorldHandle&)>([](lua_State *l,CWorldHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		auto bspTree = hEnt->GetBSPTree();
		if(bspTree == nullptr)
			return;
		Lua::Push(l,bspTree);
	}));

	Lua::Render::register_class(l,entsMod);
	Lua::ModelDef::register_class(l,entsMod);
	Lua::Animated::register_class(l,entsMod);
	Lua::Flex::register_class(l,entsMod);
	Lua::BSP::register_class(l,entsMod,defCWorld);
	Lua::Lightmap::register_class(l,entsMod);
	Lua::VertexAnimated::register_class(l,entsMod);
	Lua::SoundEmitter::register_class(l,entsMod);
	entsMod[defCWorld];

	auto &componentManager = engine->GetNetworkState(l)->GetGameState()->GetEntityComponentManager();

	auto defCToggle = luabind::class_<CToggleHandle,BaseEntityComponentHandle>("ToggleComponent");
	Lua::register_base_toggle_component_methods<luabind::class_<CToggleHandle,BaseEntityComponentHandle>,CToggleHandle>(l,defCToggle);
	entsMod[defCToggle];

	auto defCTransform = luabind::class_<CTransformHandle,BaseEntityComponentHandle>("TransformComponent");
	Lua::register_base_transform_component_methods<luabind::class_<CTransformHandle,BaseEntityComponentHandle>,CTransformHandle>(l,defCTransform);
	entsMod[defCTransform];

	auto defCWheel = luabind::class_<CWheelHandle,BaseEntityComponentHandle>("WheelComponent");
	Lua::register_base_wheel_component_methods<luabind::class_<CWheelHandle,BaseEntityComponentHandle>,CWheelHandle>(l,defCWheel);
	entsMod[defCWheel];

	auto defCSoundDsp = luabind::class_<CSoundDspHandle,BaseEntityComponentHandle>("SoundDspComponent");
	Lua::register_base_env_sound_dsp_component_methods<luabind::class_<CSoundDspHandle,BaseEntityComponentHandle>,CSoundDspHandle>(l,defCSoundDsp);
	entsMod[defCSoundDsp];

	auto defCSoundDspChorus = luabind::class_<CSoundDspChorusHandle,BaseEntityComponentHandle>("SoundDspChorusComponent");
	entsMod[defCSoundDspChorus];

	auto defCSoundDspDistortion = luabind::class_<CSoundDspDistortionHandle,BaseEntityComponentHandle>("SoundDspDistortionComponent");
	entsMod[defCSoundDspDistortion];

	auto defCSoundDspEAXReverb = luabind::class_<CSoundDspEAXReverbHandle,BaseEntityComponentHandle>("SoundDspEAXReverbComponent");
	entsMod[defCSoundDspEAXReverb];

	auto defCSoundDspEcho = luabind::class_<CSoundDspEchoHandle,BaseEntityComponentHandle>("SoundDspEchoComponent");
	entsMod[defCSoundDspEcho];

	auto defCSoundDspEqualizer = luabind::class_<CSoundDspEqualizerHandle,BaseEntityComponentHandle>("SoundDspEqualizerComponent");
	entsMod[defCSoundDspEqualizer];

	auto defCSoundDspFlanger = luabind::class_<CSoundDspFlangerHandle,BaseEntityComponentHandle>("SoundDspFlangerComponent");
	entsMod[defCSoundDspFlanger];

	auto defCCamera = luabind::class_<CCameraHandle,BaseEntityComponentHandle>("CameraComponent");
	Lua::register_base_env_camera_component_methods<luabind::class_<CCameraHandle,BaseEntityComponentHandle>,CCameraHandle>(l,defCCamera);
	entsMod[defCCamera];

	auto defCDecal = luabind::class_<CDecalHandle,BaseEntityComponentHandle>("DecalComponent");
	Lua::register_base_decal_component_methods<luabind::class_<CDecalHandle,BaseEntityComponentHandle>,CDecalHandle>(l,defCDecal);
	defCDecal.def("CreateFromProjection",static_cast<void(*)(lua_State*,CDecalHandle&,luabind::object,const pragma::physics::ScaledTransform&)>(&Lua::Decal::create_from_projection));
	defCDecal.def("CreateFromProjection",static_cast<void(*)(lua_State*,CDecalHandle&,luabind::object)>(&Lua::Decal::create_from_projection));
	defCDecal.def("DebugDraw",static_cast<void(*)(lua_State*,CDecalHandle&,float)>([](lua_State *l,CDecalHandle &hEnt,float duration) {
		pragma::Lua::check_component(l,hEnt);
		hEnt->GetProjector().DebugDraw(duration);
	}));
	entsMod[defCDecal];

	auto defCExplosion = luabind::class_<CExplosionHandle,BaseEntityComponentHandle>("ExplosionComponent");
	Lua::register_base_env_explosion_component_methods<luabind::class_<CExplosionHandle,BaseEntityComponentHandle>,CExplosionHandle>(l,defCExplosion);
	entsMod[defCExplosion];

	auto defCFire = luabind::class_<CFireHandle,BaseEntityComponentHandle>("FireComponent");
	Lua::register_base_env_fire_component_methods<luabind::class_<CFireHandle,BaseEntityComponentHandle>,CFireHandle>(l,defCFire);
	entsMod[defCFire];

	auto defCFogController = luabind::class_<CFogControllerHandle,BaseEntityComponentHandle>("FogControllerComponent");
	Lua::register_base_env_fog_controller_component_methods<luabind::class_<CFogControllerHandle,BaseEntityComponentHandle>,CFogControllerHandle>(l,defCFogController);
	entsMod[defCFogController];

	auto defCLight = luabind::class_<CLightHandle,BaseEntityComponentHandle>("LightComponent");
	Lua::register_base_env_light_component_methods<luabind::class_<CLightHandle,BaseEntityComponentHandle>,CLightHandle>(l,defCLight);
	defCLight.def("SetShadowType",static_cast<void(*)(lua_State*,CLightHandle&,uint32_t)>([](lua_State *l,CLightHandle &hComponent,uint32_t type) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetShadowType(static_cast<pragma::BaseEnvLightComponent::ShadowType>(type));
	}));
	defCLight.def("GetShadowType",static_cast<void(*)(lua_State*,CLightHandle&)>([](lua_State *l,CLightHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		Lua::PushInt(l,umath::to_integral(hComponent->GetShadowType()));
	}));
	defCLight.def("UpdateBuffers",static_cast<void(*)(lua_State*,CLightHandle&)>([](lua_State *l,CLightHandle &hComponent) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->UpdateBuffers();
	}));
	defCLight.def("SetAddToGameScene",static_cast<void(*)(lua_State*,CLightHandle&,bool)>([](lua_State *l,CLightHandle &hComponent,bool b) {
		pragma::Lua::check_component(l,hComponent);
		hComponent->SetStateFlag(pragma::CLightComponent::StateFlags::AddToGameScene,b);
	}));
	defCLight.add_static_constant("SHADOW_TYPE_NONE",umath::to_integral(ShadowType::None));
	defCLight.add_static_constant("SHADOW_TYPE_STATIC_ONLY",umath::to_integral(ShadowType::StaticOnly));
	defCLight.add_static_constant("SHADOW_TYPE_FULL",umath::to_integral(ShadowType::Full));

	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY",pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_ENTITY_MESH",pragma::CLightComponent::EVENT_SHOULD_PASS_ENTITY_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_PASS_MESH",pragma::CLightComponent::EVENT_SHOULD_PASS_MESH);
	defCLight.add_static_constant("EVENT_SHOULD_UPDATE_RENDER_PASS",pragma::CLightComponent::EVENT_SHOULD_UPDATE_RENDER_PASS);
	defCLight.add_static_constant("EVENT_GET_TRANSFORMATION_MATRIX",pragma::CLightComponent::EVENT_GET_TRANSFORMATION_MATRIX);
	defCLight.add_static_constant("EVENT_HANDLE_SHADOW_MAP",pragma::CLightComponent::EVENT_HANDLE_SHADOW_MAP);
	defCLight.add_static_constant("EVENT_ON_SHADOW_BUFFER_INITIALIZED",pragma::CLightComponent::EVENT_ON_SHADOW_BUFFER_INITIALIZED);
	entsMod[defCLight];

	auto defCLightDirectional = luabind::class_<CLightDirectionalHandle,BaseEntityComponentHandle>("LightDirectionalComponent");
	Lua::register_base_env_light_directional_component_methods<luabind::class_<CLightDirectionalHandle,BaseEntityComponentHandle>,CLightDirectionalHandle>(l,defCLightDirectional);
	entsMod[defCLightDirectional];

	auto defCLightPoint = luabind::class_<CLightPointHandle,BaseEntityComponentHandle>("LightPointComponent");
	Lua::register_base_env_light_point_component_methods<luabind::class_<CLightPointHandle,BaseEntityComponentHandle>,CLightPointHandle>(l,defCLightPoint);
	entsMod[defCLightPoint];

	auto defCLightSpot = luabind::class_<CLightSpotHandle,BaseEntityComponentHandle>("LightSpotComponent");
	Lua::register_base_env_light_spot_component_methods<luabind::class_<CLightSpotHandle,BaseEntityComponentHandle>,CLightSpotHandle>(l,defCLightSpot);
	entsMod[defCLightSpot];

	auto defCLightSpotVol = luabind::class_<CLightSpotVolHandle,BaseEntityComponentHandle>("LightSpotVolComponent");
	Lua::register_base_env_light_spot_vol_component_methods<luabind::class_<CLightSpotVolHandle,BaseEntityComponentHandle>,CLightSpotVolHandle>(l,defCLightSpotVol);
	entsMod[defCLightSpotVol];

	auto defCMicrophone = luabind::class_<CMicrophoneHandle,BaseEntityComponentHandle>("MicrophoneComponent");
	Lua::register_base_env_microphone_component_methods<luabind::class_<CMicrophoneHandle,BaseEntityComponentHandle>,CMicrophoneHandle>(l,defCMicrophone);
	entsMod[defCMicrophone];

	Lua::ParticleSystem::register_class(l,entsMod);

	auto defCQuake = luabind::class_<CQuakeHandle,BaseEntityComponentHandle>("QuakeComponent");
	Lua::register_base_env_quake_component_methods<luabind::class_<CQuakeHandle,BaseEntityComponentHandle>,CQuakeHandle>(l,defCQuake);
	entsMod[defCQuake];

	auto defCSmokeTrail = luabind::class_<CSmokeTrailHandle,BaseEntityComponentHandle>("SmokeTrailComponent");
	Lua::register_base_env_smoke_trail_component_methods<luabind::class_<CSmokeTrailHandle,BaseEntityComponentHandle>,CSmokeTrailHandle>(l,defCSmokeTrail);
	entsMod[defCSmokeTrail];

	auto defCSound = luabind::class_<CSoundHandle,BaseEntityComponentHandle>("SoundComponent");
	Lua::register_base_env_sound_component_methods<luabind::class_<CSoundHandle,BaseEntityComponentHandle>,CSoundHandle>(l,defCSound);
	entsMod[defCSound];

	auto defCSoundScape = luabind::class_<CSoundScapeHandle,BaseEntityComponentHandle>("SoundScapeComponent");
	Lua::register_base_env_soundscape_component_methods<luabind::class_<CSoundScapeHandle,BaseEntityComponentHandle>,CSoundScapeHandle>(l,defCSoundScape);
	entsMod[defCSoundScape];

	auto defCSprite = luabind::class_<CSpriteHandle,BaseEntityComponentHandle>("SpriteComponent");
	Lua::register_base_env_sprite_component_methods<luabind::class_<CSpriteHandle,BaseEntityComponentHandle>,CSpriteHandle>(l,defCSprite);
	defCSprite.def("StopAndRemoveEntity",static_cast<void(*)(lua_State*,CSpriteHandle&)>([](lua_State *l,CSpriteHandle &hSprite) {
		pragma::Lua::check_component(l,hSprite);
		hSprite->StopAndRemoveEntity();
	}));
	entsMod[defCSprite];

	auto defCTimescale = luabind::class_<CEnvTimescaleHandle,BaseEntityComponentHandle>("EnvTimescaleComponent");
	Lua::register_base_env_timescale_component_methods<luabind::class_<CEnvTimescaleHandle,BaseEntityComponentHandle>,CEnvTimescaleHandle>(l,defCTimescale);
	entsMod[defCTimescale];

	auto defCWind = luabind::class_<CWindHandle,BaseEntityComponentHandle>("WindComponent");
	Lua::register_base_env_wind_component_methods<luabind::class_<CWindHandle,BaseEntityComponentHandle>,CWindHandle>(l,defCWind);
	entsMod[defCWind];

	auto defCFilterClass = luabind::class_<CFilterClassHandle,BaseEntityComponentHandle>("FilterClassComponent");
	Lua::register_base_env_filter_class_component_methods<luabind::class_<CFilterClassHandle,BaseEntityComponentHandle>,CFilterClassHandle>(l,defCFilterClass);
	entsMod[defCFilterClass];

	auto defCFilterName = luabind::class_<CFilterNameHandle,BaseEntityComponentHandle>("FilterNameComponent");
	Lua::register_base_env_filter_name_component_methods<luabind::class_<CFilterNameHandle,BaseEntityComponentHandle>,CFilterNameHandle>(l,defCFilterName);
	entsMod[defCFilterName];

	auto defCBrush = luabind::class_<CBrushHandle,BaseEntityComponentHandle>("BrushComponent");
	Lua::register_base_func_brush_component_methods<luabind::class_<CBrushHandle,BaseEntityComponentHandle>,CBrushHandle>(l,defCBrush);
	entsMod[defCBrush];

	auto defCKinematic = luabind::class_<CKinematicHandle,BaseEntityComponentHandle>("KinematicComponent");
	Lua::register_base_func_kinematic_component_methods<luabind::class_<CKinematicHandle,BaseEntityComponentHandle>,CKinematicHandle>(l,defCKinematic);
	entsMod[defCKinematic];

	auto defCFuncPhysics = luabind::class_<CFuncPhysicsHandle,BaseEntityComponentHandle>("FuncPhysicsComponent");
	Lua::register_base_func_physics_component_methods<luabind::class_<CFuncPhysicsHandle,BaseEntityComponentHandle>,CFuncPhysicsHandle>(l,defCFuncPhysics);
	entsMod[defCFuncPhysics];

	auto defCFuncSoftPhysics = luabind::class_<CFuncSoftPhysicsHandle,BaseEntityComponentHandle>("FuncSoftPhysicsComponent");
	Lua::register_base_func_soft_physics_component_methods<luabind::class_<CFuncSoftPhysicsHandle,BaseEntityComponentHandle>,CFuncSoftPhysicsHandle>(l,defCFuncSoftPhysics);
	entsMod[defCFuncSoftPhysics];

	auto defCFuncPortal = luabind::class_<CFuncPortalHandle,BaseEntityComponentHandle>("FuncPortalComponent");
	Lua::register_base_func_portal_component_methods<luabind::class_<CFuncPortalHandle,BaseEntityComponentHandle>,CFuncPortalHandle>(l,defCFuncPortal);
	entsMod[defCFuncPortal];

	auto defCWater = luabind::class_<CWaterHandle,BaseEntityComponentHandle>("WaterComponent");
	Lua::register_base_func_water_component_methods<luabind::class_<CWaterHandle,BaseEntityComponentHandle>,CWaterHandle>(l,defCWater);
	defCWater.def("GetReflectionScene",static_cast<void(*)(lua_State*,CWaterHandle&)>([](lua_State *l,CWaterHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		if(hEnt->IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<Scene>>(l,hEnt->GetWaterScene().sceneReflection);
	}));
	defCWater.def("GetWaterSceneTexture",static_cast<void(*)(lua_State*,CWaterHandle&)>([](lua_State *l,CWaterHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		if(hEnt->IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,hEnt->GetWaterScene().texScene);
	}));
	defCWater.def("GetWaterSceneDepthTexture",static_cast<void(*)(lua_State*,CWaterHandle&)>([](lua_State *l,CWaterHandle &hEnt) {
		pragma::Lua::check_component(l,hEnt);
		if(hEnt->IsWaterSceneValid() == false)
			return;
		Lua::Push<std::shared_ptr<prosper::Texture>>(l,hEnt->GetWaterScene().texSceneDepth);
	}));
	entsMod[defCWater];

	auto defCButton = luabind::class_<CButtonHandle,BaseEntityComponentHandle>("ButtonComponent");
	Lua::register_base_func_button_component_methods<luabind::class_<CButtonHandle,BaseEntityComponentHandle>,CButtonHandle>(l,defCButton);
	entsMod[defCButton];

	auto defCBot = luabind::class_<CBotHandle,BaseEntityComponentHandle>("BotComponent");
	Lua::register_base_bot_component_methods<luabind::class_<CBotHandle,BaseEntityComponentHandle>,CBotHandle>(l,defCBot);
	entsMod[defCBot];

	auto defCPointConstraintBallSocket = luabind::class_<CPointConstraintBallSocketHandle,BaseEntityComponentHandle>("PointConstraintBallSocketComponent");
	Lua::register_base_point_constraint_ball_socket_component_methods<luabind::class_<CPointConstraintBallSocketHandle,BaseEntityComponentHandle>,CPointConstraintBallSocketHandle>(l,defCPointConstraintBallSocket);
	entsMod[defCPointConstraintBallSocket];

	auto defCPointConstraintConeTwist = luabind::class_<CPointConstraintConeTwistHandle,BaseEntityComponentHandle>("PointConstraintConeTwistComponent");
	Lua::register_base_point_constraint_cone_twist_component_methods<luabind::class_<CPointConstraintConeTwistHandle,BaseEntityComponentHandle>,CPointConstraintConeTwistHandle>(l,defCPointConstraintConeTwist);
	entsMod[defCPointConstraintConeTwist];

	auto defCPointConstraintDoF = luabind::class_<CPointConstraintDoFHandle,BaseEntityComponentHandle>("PointConstraintDoFComponent");
	Lua::register_base_point_constraint_dof_component_methods<luabind::class_<CPointConstraintDoFHandle,BaseEntityComponentHandle>,CPointConstraintDoFHandle>(l,defCPointConstraintDoF);
	entsMod[defCPointConstraintDoF];

	auto defCPointConstraintFixed = luabind::class_<CPointConstraintFixedHandle,BaseEntityComponentHandle>("PointConstraintFixedComponent");
	Lua::register_base_point_constraint_fixed_component_methods<luabind::class_<CPointConstraintFixedHandle,BaseEntityComponentHandle>,CPointConstraintFixedHandle>(l,defCPointConstraintFixed);
	entsMod[defCPointConstraintFixed];

	auto defCPointConstraintHinge = luabind::class_<CPointConstraintHingeHandle,BaseEntityComponentHandle>("PointConstraintHingeComponent");
	Lua::register_base_point_constraint_hinge_component_methods<luabind::class_<CPointConstraintHingeHandle,BaseEntityComponentHandle>,CPointConstraintHingeHandle>(l,defCPointConstraintHinge);
	entsMod[defCPointConstraintHinge];

	auto defCPointConstraintSlider = luabind::class_<CPointConstraintSliderHandle,BaseEntityComponentHandle>("PointConstraintSliderComponent");
	Lua::register_base_point_constraint_slider_component_methods<luabind::class_<CPointConstraintSliderHandle,BaseEntityComponentHandle>,CPointConstraintSliderHandle>(l,defCPointConstraintSlider);
	entsMod[defCPointConstraintSlider];

	auto defCRenderTarget = luabind::class_<CRenderTargetHandle,BaseEntityComponentHandle>("RenderTargetComponent");
	Lua::register_base_point_render_target_component_methods<luabind::class_<CRenderTargetHandle,BaseEntityComponentHandle>,CRenderTargetHandle>(l,defCRenderTarget);
	entsMod[defCRenderTarget];

	auto defCPointTarget = luabind::class_<CPointTargetHandle,BaseEntityComponentHandle>("PointTargetComponent");
	Lua::register_base_point_target_component_methods<luabind::class_<CPointTargetHandle,BaseEntityComponentHandle>,CPointTargetHandle>(l,defCPointTarget);
	entsMod[defCPointTarget];

	auto defCProp = luabind::class_<CPropHandle,BaseEntityComponentHandle>("PropComponent");
	Lua::register_base_prop_component_methods<luabind::class_<CPropHandle,BaseEntityComponentHandle>,CPropHandle>(l,defCProp);
	entsMod[defCProp];

	auto defCPropDynamic = luabind::class_<CPropDynamicHandle,BaseEntityComponentHandle>("PropDynamicComponent");
	Lua::register_base_prop_dynamic_component_methods<luabind::class_<CPropDynamicHandle,BaseEntityComponentHandle>,CPropDynamicHandle>(l,defCPropDynamic);
	entsMod[defCPropDynamic];

	auto defCPropPhysics = luabind::class_<CPropPhysicsHandle,BaseEntityComponentHandle>("PropPhysicsComponent");
	Lua::register_base_prop_physics_component_methods<luabind::class_<CPropPhysicsHandle,BaseEntityComponentHandle>,CPropPhysicsHandle>(l,defCPropPhysics);
	entsMod[defCPropPhysics];

	auto defCTouch = luabind::class_<CTouchHandle,BaseEntityComponentHandle>("TouchComponent");
	Lua::register_base_touch_component_methods<luabind::class_<CTouchHandle,BaseEntityComponentHandle>,CTouchHandle>(l,defCTouch);
	entsMod[defCTouch];

	auto defCSkybox = luabind::class_<CSkyboxHandle,BaseEntityComponentHandle>("SkyboxComponent");
	Lua::register_base_skybox_component_methods<luabind::class_<CSkyboxHandle,BaseEntityComponentHandle>,CSkyboxHandle>(l,defCSkybox);
	entsMod[defCSkybox];

	auto defCFlashlight = luabind::class_<CFlashlightHandle,BaseEntityComponentHandle>("FlashlightComponent");
	Lua::register_base_flashlight_component_methods<luabind::class_<CFlashlightHandle,BaseEntityComponentHandle>,CFlashlightHandle>(l,defCFlashlight);
	entsMod[defCFlashlight];

	auto defCEnvSoundProbe = luabind::class_<CEnvSoundProbeHandle,BaseEntityComponentHandle>("EnvSoundProbeComponent");
	entsMod[defCEnvSoundProbe];

	auto defCWeather = luabind::class_<CWeatherHandle,BaseEntityComponentHandle>("WeatherComponent");
	Lua::register_base_env_weather_component_methods<luabind::class_<CWeatherHandle,BaseEntityComponentHandle>,CWeatherHandle>(l,defCWeather);
	entsMod[defCWeather];

	auto defCReflectionProbe = luabind::class_<CReflectionProbeHandle,BaseEntityComponentHandle>("ReflectionProbeComponent");
	entsMod[defCReflectionProbe];

	auto defCPBRConverter = luabind::class_<CPBRConverterHandle,BaseEntityComponentHandle>("PBRConverterComponent");
	defCPBRConverter.def("GenerateAmbientOcclusionMaps",static_cast<void(*)(lua_State*,CPBRConverterHandle&,Model&)>([](lua_State *l,CPBRConverterHandle &hEnt,Model &mdl) {
		pragma::Lua::check_component(l,hEnt);
		hEnt->GenerateAmbientOcclusionMaps(mdl);
	}));
	entsMod[defCPBRConverter];

	auto defShadow = luabind::class_<CShadowHandle,BaseEntityComponentHandle>("ShadowMapComponent");
	entsMod[defShadow];

	auto defShadowCsm = luabind::class_<CShadowCSMHandle,BaseEntityComponentHandle>("CSMComponent");
	entsMod[defShadowCsm];

	auto defShadowManager = luabind::class_<CShadowManagerHandle,BaseEntityComponentHandle>("ShadowManagerComponent");
	entsMod[defShadowManager];

	auto defCWaterSurface = luabind::class_<CWaterSurfaceHandle,BaseEntityComponentHandle>("WaterSurfaceComponent");
	entsMod[defCWaterSurface];

	auto defCListener = luabind::class_<CListenerHandle,BaseEntityComponentHandle>("ListenerComponent");
	entsMod[defCListener];

	auto defCViewBody = luabind::class_<CViewBodyHandle,BaseEntityComponentHandle>("ViewBodyComponent");
	entsMod[defCViewBody];

	auto defCViewModel = luabind::class_<CViewModelHandle,BaseEntityComponentHandle>("ViewModelComponent");
	entsMod[defCViewModel];

	auto defCSoftBody = luabind::class_<CSoftBodyHandle,BaseEntityComponentHandle>("SoftBodyComponent");
	entsMod[defCSoftBody];

	auto defCRaytracing = luabind::class_<CRaytracingHandle,BaseEntityComponentHandle>("RaytracingComponent");
	entsMod[defCRaytracing];

	auto defCBSPLeaf = luabind::class_<CBSPLeafHandle,BaseEntityComponentHandle>("BSPLeafComponent");
	entsMod[defCBSPLeaf];

	auto defCIo = luabind::class_<CIOHandle,BaseEntityComponentHandle>("IOComponent");
	Lua::register_base_io_component_methods<luabind::class_<CIOHandle,BaseEntityComponentHandle>,CIOHandle>(l,defCIo);
	entsMod[defCIo];

	auto defCTimeScale = luabind::class_<CTimeScaleHandle,BaseEntityComponentHandle>("TimeScaleComponent");
	Lua::register_base_time_scale_component_methods<luabind::class_<CTimeScaleHandle,BaseEntityComponentHandle>,CTimeScaleHandle>(l,defCTimeScale);
	entsMod[defCTimeScale];

	auto defCAttachable = luabind::class_<CAttachableHandle,BaseEntityComponentHandle>("AttachableComponent");
	Lua::register_base_attachable_component_methods<luabind::class_<CAttachableHandle,BaseEntityComponentHandle>,CAttachableHandle>(l,defCAttachable);
	entsMod[defCAttachable];

	auto defCParent = luabind::class_<CParentHandle,BaseEntityComponentHandle>("ParentComponent");
	Lua::register_base_parent_component_methods<luabind::class_<CParentHandle,BaseEntityComponentHandle>,CParentHandle>(l,defCParent);
	entsMod[defCParent];
	
	auto defCOwnable = luabind::class_<COwnableHandle,BaseEntityComponentHandle>("OwnableComponent");
	Lua::register_base_ownable_component_methods<luabind::class_<COwnableHandle,BaseEntityComponentHandle>,COwnableHandle>(l,defCOwnable);
	entsMod[defCOwnable];

	auto defCDebugText = luabind::class_<CDebugTextHandle,BaseEntityComponentHandle>("DebugTextComponent");
	Lua::register_base_debug_text_component_methods<luabind::class_<CDebugTextHandle,BaseEntityComponentHandle>,CDebugTextHandle>(l,defCDebugText);
	entsMod[defCDebugText];

	auto defCDebugPoint = luabind::class_<CDebugPointHandle,BaseEntityComponentHandle>("DebugPointComponent");
	Lua::register_base_debug_point_component_methods<luabind::class_<CDebugPointHandle,BaseEntityComponentHandle>,CDebugPointHandle>(l,defCDebugPoint);
	entsMod[defCDebugPoint];

	auto defCDebugLine = luabind::class_<CDebugLineHandle,BaseEntityComponentHandle>("DebugLineComponent");
	Lua::register_base_debug_line_component_methods<luabind::class_<CDebugLineHandle,BaseEntityComponentHandle>,CDebugLineHandle>(l,defCDebugLine);
	entsMod[defCDebugLine];

	auto defCDebugBox = luabind::class_<CDebugBoxHandle,BaseEntityComponentHandle>("DebugBoxComponent");
	Lua::register_base_debug_box_component_methods<luabind::class_<CDebugBoxHandle,BaseEntityComponentHandle>,CDebugBoxHandle>(l,defCDebugBox);
	entsMod[defCDebugBox];

	auto defCDebugSphere = luabind::class_<CDebugSphereHandle,BaseEntityComponentHandle>("DebugSphereComponent");
	Lua::register_base_debug_sphere_component_methods<luabind::class_<CDebugSphereHandle,BaseEntityComponentHandle>,CDebugSphereHandle>(l,defCDebugSphere);
	entsMod[defCDebugSphere];

	auto defCDebugCone = luabind::class_<CDebugConeHandle,BaseEntityComponentHandle>("DebugConeComponent");
	Lua::register_base_debug_cone_component_methods<luabind::class_<CDebugConeHandle,BaseEntityComponentHandle>,CDebugConeHandle>(l,defCDebugCone);
	entsMod[defCDebugCone];

	auto defCDebugCylinder = luabind::class_<CDebugCylinderHandle,BaseEntityComponentHandle>("DebugCylinderComponent");
	Lua::register_base_debug_cylinder_component_methods<luabind::class_<CDebugCylinderHandle,BaseEntityComponentHandle>,CDebugCylinderHandle>(l,defCDebugCylinder);
	entsMod[defCDebugCylinder];

	auto defCDebugPlane = luabind::class_<CDebugPlaneHandle,BaseEntityComponentHandle>("DebugPlaneComponent");
	Lua::register_base_debug_plane_component_methods<luabind::class_<CDebugPlaneHandle,BaseEntityComponentHandle>,CDebugPlaneHandle>(l,defCDebugPlane);
	entsMod[defCDebugPlane];

	auto defCPointAtTarget = luabind::class_<CPointAtTargetHandle,BaseEntityComponentHandle>("PointAtTargetComponent");
	Lua::register_base_point_at_target_component_methods<luabind::class_<CPointAtTargetHandle,BaseEntityComponentHandle>,CPointAtTargetHandle>(l,defCPointAtTarget);
	entsMod[defCPointAtTarget];

	auto defCBSP = luabind::class_<CBSPHandle,BaseEntityComponentHandle>("BSPComponent");
	entsMod[defCBSP];

	auto defCGeneric = luabind::class_<CGenericHandle,BaseEntityComponentHandle>("EntityComponent");
	//Lua::register_base_generic_component_methods<luabind::class_<CGenericHandle,BaseEntityComponentHandle>,CGenericHandle>(l,defCGeneric);
	entsMod[defCGeneric];
}

//////////////

void Lua::Flex::GetFlexController(lua_State *l,CFlexHandle &hEnt,uint32_t flexId)
{
	pragma::Lua::check_component(l,hEnt);
	auto val = 0.f;
	if(hEnt->GetFlexController(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}
void Lua::Flex::GetFlexController(lua_State *l,CFlexHandle &hEnt,const std::string &flexController)
{
	pragma::Lua::check_component(l,hEnt);
	auto flexId = 0u;
	auto mdlComponent = hEnt->GetEntity().GetModelComponent();
	if(mdlComponent.expired() || mdlComponent->LookupFlexController(flexController,flexId) == false)
		return;
	auto val = 0.f;
	if(hEnt->GetFlexController(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}
void Lua::Flex::CalcFlexValue(lua_State *l,CFlexHandle &hEnt,uint32_t flexId)
{
	pragma::Lua::check_component(l,hEnt);
	auto val = 0.f;
	if(hEnt->CalcFlexValue(flexId,val) == false)
		return;
	Lua::PushNumber(l,val);
}

//////////////

void Lua::SoundEmitter::CreateSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->CreateSound(sndname,static_cast<ALSoundType>(soundType));
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}
void Lua::SoundEmitter::EmitSound(lua_State *l,CSoundEmitterHandle &hEnt,std::string sndname,uint32_t soundType,float gain,float pitch,bool bTransmit)
{
	pragma::Lua::check_component(l,hEnt);
	auto snd = hEnt->EmitSound(sndname,static_cast<ALSoundType>(soundType),gain,pitch);
	if(snd == nullptr)
		return;
	luabind::object(l,snd).push(l);
}

//////////////

void Lua::ParticleSystem::Stop(lua_State *l,CParticleSystemHandle &hComponent,bool bStopImmediately)
{
	pragma::Lua::check_component(l,hComponent);
	if(bStopImmediately == true)
		hComponent->Stop();
	else
		hComponent->Die();
}
void Lua::ParticleSystem::AddInitializer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::CheckString(l,-3);
		std::string val = Lua::ParticleSystem::get_key_value(l,-2);
		StringToLower(key);
		values[key] = val;
	}
	hComponent.AddInitializer(name,values);
}
void Lua::ParticleSystem::AddOperator(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::CheckString(l,-3);
		std::string val = Lua::ParticleSystem::get_key_value(l,-2);
		StringToLower(key);
		values[key] = val;
	}
	hComponent.AddOperator(name,values);
}
void Lua::ParticleSystem::AddRenderer(lua_State *l,pragma::CParticleSystemComponent &hComponent,std::string name,luabind::object o)
{
	auto t = Lua::GetStackTop(l);
	std::unordered_map<std::string,std::string> values;
	Lua::CheckTable(l,t);
	Lua::PushNil(l);
	while(Lua::GetNextPair(l,t) != 0)
	{
		Lua::PushValue(l,-2);
		std::string key = Lua::CheckString(l,-3);
		std::string val = Lua::ParticleSystem::get_key_value(l,-2);
		StringToLower(key);
		values[key] = val;
	}
	hComponent.AddRenderer(name,values);
}
