#include "pch.h"
#include "Controller.h"

using namespace DirectX;
using namespace Windows::Devices::Input;
using namespace Windows::UI::Core;
using namespace Windows::UI::Input;
using namespace Windows::Foundation;

Controller::Controller(CoreWindow ^window)
{
	InitWindow(window);
}

void Controller::InitWindow(CoreWindow ^window)
{
#ifdef _DEBUG
	DebugTrace(L"Init Controller\n");
#endif // DEBUG

	ResetState();

	window->KeyDown += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &Controller::OnKeyDown);
	window->KeyUp += ref new TypedEventHandler<CoreWindow ^, KeyEventArgs ^>(this, &Controller::OnKeyUp);
}

void Controller::Update()
{
	XMFLOAT3 m_moveCommand{0, 0, 0};

	// 왼손 좌표계
	// X 오른쪽, Y 위, Z 화면 안쪽
	if (m_forward)
		m_moveCommand.z += 1.0f;
	if (m_back)
		m_moveCommand.z -= 1.0f;
	if (m_left)
		m_moveCommand.x -= 1.0f;
	if (m_right)
		m_moveCommand.x += 1.0f;

	if (fabsf(m_moveCommand.x) > 0.1f ||
		fabsf(m_moveCommand.y) > 0.1f ||
		fabsf(m_moveCommand.z) > 0.1f)
	{
		XMStoreFloat3(&m_moveCommand, XMVector3Normalize(XMLoadFloat3(&m_moveCommand)));
	}

	m_velocity.x = m_moveCommand.x * MoveConstant::Movement;
	m_velocity.y = m_moveCommand.y * MoveConstant::Movement;
	m_velocity.z = m_moveCommand.z * MoveConstant::Movement;

//#ifdef _DEBUG
//	DebugTrace(L"x : %f, y : %f, z : %f\n", m_velocity.x, m_velocity.y, m_velocity.z);
//#endif // DEBUG
}

void Controller::ResetState()
{
	m_velocity = XMFLOAT3(0, 0, 0);
	m_forward = false;
	m_back = false;
	m_left = false;
	m_right = false;
}

void Controller::OnKeyDown(CoreWindow ^sender, KeyEventArgs ^args)
{
#ifdef _DEBUG
	DebugTrace(L"KeyDown\n");
#endif // DEBUG
	using namespace Windows::System;
	
	switch (VirtualKey Key = args->VirtualKey; Key)
	{
	case (VirtualKey::Up):
		m_forward = true;
		break;
	case (VirtualKey::Down):
		m_back = true;
		break;
	case (VirtualKey::Left):
		m_left = true;
		break;
	case (VirtualKey::Right):
		m_right = true;
		break;

	default:
		break;
	}
}

void Controller::OnKeyUp(CoreWindow ^sender, KeyEventArgs ^args)
{
#ifdef _DEBUG
	DebugTrace(L"KeyUp\n");
#endif // DEBUG
	using namespace Windows::System;

	switch (VirtualKey Key = args->VirtualKey; Key)
	{
	case (VirtualKey::Up):
		m_forward = false;
		break;
	case (VirtualKey::Down):
		m_back = false;
		break;
	case (VirtualKey::Left):
		m_left = false;
		break;
	case (VirtualKey::Right):
		m_right = false;
		break;

	default:
		break;
	}
}


//----------------------------------------------------------------------------------------

#ifdef _DEBUG
void Controller::DebugTrace(const wchar_t *format, ...)
{
	// Generate the message string.
	va_list args;
	va_start(args, format);
	wchar_t message[1024];
	vswprintf_s(message, 1024, format, args);
	OutputDebugStringW(message);
}
#endif // DEBUG