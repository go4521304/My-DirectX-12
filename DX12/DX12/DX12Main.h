﻿#pragma once

#include "Common\StepTimer.h"
#include "Common\DeviceResources.h"
#include "Content\Sample3DSceneRenderer.h"
#include "Controller.h"

// 화면에서 Direct3D 콘텐츠를 렌더링합니다.
namespace DX12
{
	class DX12Main
	{
	public:
		DX12Main();
		void CreateRenderers(const std::shared_ptr<DX::DeviceResources>& deviceResources);
		void Update();
		bool Render();

		void OnWindowSizeChanged();
		void OnSuspending();
		void OnResuming();
		void OnDeviceRemoved();

	private:
		// Controller
		Controller ^m_controller;

		// TODO: 사용자 콘텐츠 렌더러로 대체합니다.
		std::unique_ptr<Sample3DSceneRenderer> m_sceneRenderer;

		// 렌더링 루프 타이머입니다.
		DX::StepTimer m_timer;
	};
}