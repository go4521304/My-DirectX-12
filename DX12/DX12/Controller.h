#pragma once

#include "pch.h"
#include "Common\DeviceResources.h"

namespace MoveConstant
{
	constexpr float Movement = 2.0f;
}

class Command
{
public:
	virtual ~Command() {}
	virtual void execute() = 0;
};

enum class ControllerState
{
	None,
	Active,
};

ref class Controller
{
internal:
	Controller(_In_ Windows::UI::Core::CoreWindow ^window);
	void InitWindow(_In_ Windows::UI::Core::CoreWindow ^window);

	inline DirectX::XMFLOAT3 Velocity() { return m_velocity; };

	void Update();

#ifdef _DEBUG
	void DebugTrace(const wchar_t *format, ...);
#endif // DEBUG


private:
	void ResetState();

	void OnKeyDown(_In_ Windows::UI::Core::CoreWindow ^sender, _In_ Windows::UI::Core::KeyEventArgs ^args);
	void OnKeyUp(_In_ Windows::UI::Core::CoreWindow ^sender, _In_ Windows::UI::Core::KeyEventArgs ^args);


private:
	//ControllerState				m_state;
	DirectX::XMFLOAT3           m_velocity;             // How far we move in this frame.
	
	// Keyboard Button
	bool						m_forward;
	bool						m_back;
	bool						m_left;
	bool						m_right;
};