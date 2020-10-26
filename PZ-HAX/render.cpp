#include "render.h"
#include "ObfuscationMgr.h"
#include "CSX.h"
#include "Configuration.hpp"
#include "direct3d.h"
#include "Color.h"
#include "Gui/Gui.h"
#include "IMem.h"
#define checkSetting(teamval, enemyval) (((pLocalPlayer->teamId == pPlayer->teamId) && teamval) || ((pLocalPlayer->teamId != pPlayer->teamId) && enemyval))
typedef void(*LPSEARCHFUNC)(LPCTSTR lpszFileName);

static class Text
{
public:
	std::string text;
	int side;
	int size;
	D3DXCOLOR color;
	Text(std::string text, int side, int size, D3DXCOLOR color) : text(text), side(side),  size(size), color(color) {}
};
namespace Render {


#pragma region utils draw

	float Distance3D(D3DXVECTOR3  v1, D3DXVECTOR3 v2)
	{
		float x_d = (v2.x - v1.x);
		float y_d = (v2.y - v1.y);
		float z_d = (v2.z - v1.z);
		return sqrt((x_d * x_d) + (y_d * y_d) + (z_d * z_d));
	}
	static D3DXMATRIXA16 s_ViewProj = D3DXMATRIXA16(0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
	bool WorldToScreen(D3DXVECTOR3& vLocVec4)
	{
		D3DXMATRIXA16 ViewProj = s_ViewProj;
		float mX = Globals::rWidth / 2;
		float mY = Globals::rHeight / 2;

		float w = ViewProj(0, 3) * vLocVec4.x + ViewProj(1, 3) * vLocVec4.y + ViewProj(2, 3) * vLocVec4.z + ViewProj(3, 3);

		if (w < 0.65f)
		{
			vLocVec4.z = w;
			return false;
		}
		float x = ViewProj(0, 0) * vLocVec4.x + ViewProj(1, 0) * vLocVec4.y + ViewProj(2, 0) * vLocVec4.z + ViewProj(3, 0);
		float y = ViewProj(0, 1) * vLocVec4.x + ViewProj(1, 1) * vLocVec4.y + ViewProj(2, 1) * vLocVec4.z + ViewProj(3, 1);
		vLocVec4.x = (mX + mX * x / w);
		vLocVec4.y = (mY - mY * y / w);
		vLocVec4.z = w;
		if (vLocVec4.x > Globals::rWidth || vLocVec4.y > Globals::rHeight || vLocVec4.x < 0 || vLocVec4.y < 0)
			return false;
		return true;
	}

	D3DXVECTOR3 Multiply(D3DXVECTOR3 vector, D3DXMATRIXA16 mat)
	{
		return  D3DXVECTOR3(mat._11 * vector.x + mat._21 * vector.y + mat._31 * vector.z,
			mat._21 * vector.x + mat._22 * vector.y + mat._32 * vector.z,
			mat._13 * vector.x + mat._23 * vector.y + mat._33 * vector.z);
	}
	void DrawAABB(fb::AxisAlignedBox aabb, D3DXVECTOR3 Position, float Yaw, D3DXCOLOR color, float thickness)
	{
		
		float cosY = (float)cos(Yaw);
		float sinY = (float)sin(Yaw);

		D3DXVECTOR3 fld = D3DXVECTOR3(aabb.Min.z * cosY - aabb.Min.x * sinY, aabb.Min.y, aabb.Min.x * cosY + aabb.Min.z * sinY) + Position; // 0
		D3DXVECTOR3 brt = D3DXVECTOR3(aabb.Min.z * cosY - aabb.Max.x * sinY, aabb.Min.y, aabb.Max.x * cosY + aabb.Min.z * sinY) + Position; // 1
		D3DXVECTOR3 bld = D3DXVECTOR3(aabb.Max.z * cosY - aabb.Max.x * sinY, aabb.Min.y, aabb.Max.x * cosY + aabb.Max.z * sinY) + Position; // 2
		D3DXVECTOR3 frt = D3DXVECTOR3(aabb.Max.z * cosY - aabb.Min.x * sinY, aabb.Min.y, aabb.Min.x * cosY + aabb.Max.z * sinY) + Position; // 3
		D3DXVECTOR3 frd = D3DXVECTOR3(aabb.Max.z * cosY - aabb.Min.x * sinY, aabb.Max.y, aabb.Min.x * cosY + aabb.Max.z * sinY) + Position; // 4
		D3DXVECTOR3 brb = D3DXVECTOR3(aabb.Min.z * cosY - aabb.Min.x * sinY, aabb.Max.y, aabb.Min.x * cosY + aabb.Min.z * sinY) + Position; // 5
		D3DXVECTOR3 blt = D3DXVECTOR3(aabb.Min.z * cosY - aabb.Max.x * sinY, aabb.Max.y, aabb.Max.x * cosY + aabb.Min.z * sinY) + Position; // 6
		D3DXVECTOR3 flt = D3DXVECTOR3(aabb.Max.z * cosY - aabb.Max.x * sinY, aabb.Max.y, aabb.Max.x * cosY + aabb.Max.z * sinY) + Position; // 7

		if (!WorldToScreen(fld) || !WorldToScreen(brt)
			|| !WorldToScreen(bld) || !WorldToScreen(frt)
			|| !WorldToScreen(frd) || !WorldToScreen(brb)
			|| !WorldToScreen(blt) || !WorldToScreen(flt))
			return;

		Renderer::GetInstance()->RenderLine(ImVec2(fld.x, fld.y), ImVec2(brt.x, brt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(brb.x, brb.y), ImVec2(blt.x, blt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(fld.x, fld.y), ImVec2(brb.x, brb.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(brt.x, brt.y), ImVec2(blt.x, blt.y),color, thickness);

		Renderer::GetInstance()->RenderLine(ImVec2(frt.x, frt.y), ImVec2(bld.x, bld.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frd.x, frd.y), ImVec2(flt.x, flt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frt.x, frt.y), ImVec2(frd.x, frd.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(bld.x, bld.y), ImVec2(flt.x, flt.y),color, thickness);

		Renderer::GetInstance()->RenderLine(ImVec2(frt.x, frt.y), ImVec2(fld.x, fld.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frd.x, frd.y), ImVec2(brb.x, brb.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(brt.x, brt.y), ImVec2(bld.x, bld.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(blt.x, blt.y), ImVec2(flt.x, flt.y),color, thickness);
		
	}
	void VinDrawAABB(fb::AxisAlignedBox aabb, D3DXMATRIXA16 tranform, D3DXCOLOR color, float thickness)
	{
	
		D3DXVECTOR3 Position = D3DXVECTOR3(tranform._41, tranform._42, tranform._43);
		D3DXVECTOR3 fld = Multiply(D3DXVECTOR3(aabb.Min.x, aabb.Min.y, aabb.Min.z), tranform) + Position;
		D3DXVECTOR3 brt = Multiply(D3DXVECTOR3(aabb.Max.x, aabb.Max.y, aabb.Max.z), tranform) + Position;
		D3DXVECTOR3 bld = Multiply(D3DXVECTOR3(aabb.Min.x, aabb.Min.y, aabb.Max.z), tranform) + Position;
		D3DXVECTOR3 frt = Multiply(D3DXVECTOR3(aabb.Max.x, aabb.Max.y, aabb.Min.z), tranform) + Position;
		D3DXVECTOR3 frd = Multiply(D3DXVECTOR3(aabb.Max.x, aabb.Min.y, aabb.Min.z), tranform) + Position;
		D3DXVECTOR3 brb = Multiply(D3DXVECTOR3(aabb.Max.x, aabb.Min.y, aabb.Max.z), tranform) + Position;
		D3DXVECTOR3 blt = Multiply(D3DXVECTOR3(aabb.Min.x, aabb.Max.y, aabb.Max.z), tranform) + Position;
		D3DXVECTOR3 flt = Multiply(D3DXVECTOR3(aabb.Min.x, aabb.Max.y, aabb.Min.z), tranform) + Position;

		if (!WorldToScreen(fld) || !WorldToScreen(brt)
			|| !WorldToScreen(bld) || !WorldToScreen(frt)
			|| !WorldToScreen(frd) || !WorldToScreen(brb)
			|| !WorldToScreen(blt) || !WorldToScreen(flt))
			return;

		Renderer::GetInstance()->RenderLine(ImVec2(fld.x, fld.y), ImVec2(flt.x, flt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(flt.x, flt.y), ImVec2(frt.x, frt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frt.x, frt.y), ImVec2(frd.x, frd.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frd.x, frd.y), ImVec2(fld.x, fld.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(bld.x, bld.y), ImVec2(blt.x, blt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(blt.x, blt.y), ImVec2(brt.x, brt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(brt.x, brt.y), ImVec2(brb.x, brb.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(brb.x, brb.y), ImVec2(bld.x, bld.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(fld.x, fld.y), ImVec2(bld.x, bld.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frd.x, frd.y), ImVec2(brb.x, brb.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(flt.x, flt.y), ImVec2(blt.x, blt.y),color, thickness);
		Renderer::GetInstance()->RenderLine(ImVec2(frt.x, frt.y), ImVec2(brt.x, brt.y),color, thickness);
		
	}
	void DrawBoneHead(D3DXVECTOR3 from, D3DXCOLOR color)
	{
		D3DXVECTOR3 W2S_from = from;
		if (!WorldToScreen(W2S_from)) return;
		Renderer::GetInstance()->RenderCircle(ImVec2(W2S_from.x, W2S_from.y), 4, color, g_Options.ESP.ESP_SETTINGS.espski);
	}
	void DrawBone(D3DXVECTOR3 from, D3DXVECTOR3 to, D3DXCOLOR color)
	{
		D3DXVECTOR3 W2S_from = from;
		if (!WorldToScreen(W2S_from)) return;
		D3DXVECTOR3  W2S_to = to;
		if (!WorldToScreen(W2S_to)) return;
		Renderer::GetInstance()->RenderLine(ImVec2(W2S_from.x, W2S_from.y), ImVec2(W2S_to.x, W2S_to.y),color, g_Options.ESP.ESP_SETTINGS.espski);
		Renderer::GetInstance()->RenderCircleFilled(ImVec2(W2S_from.x, W2S_from.y), 2,color);
	}
	void DrawSkeletonESP(fb::ClientSoldierEntity* pEnemySoldier, D3DXCOLOR Color)
	{
		D3DXVECTOR3 BonePosition[12];
		int BoneID[12] = {
						 fb::BONE_Head,				//00
						 fb::BONE_LeftHand,			//01
						 fb::BONE_RightHand,		//02
						 fb::BONE_RightFoot,		//03
						 fb::BONE_RightKneeRoll,	//04
						 fb::BONE_LeftKneeRoll,		//05
						 fb::BONE_LeftFoot,			//06
						 fb::BONE_RightShoulder,	//07
						 fb::BONE_LeftShoulder,		//08
						 fb::BONE_Spine,			//09
						 fb::BONE_RightElbowRoll,	//10
						 fb::BONE_LeftElbowRoll		//11
		};
		for (int i = 0; i < 12; i++)
		{
			if (!pEnemySoldier->GetBonePos(BoneID[i], &BonePosition[i])) return;
		}
		DrawBoneHead(BonePosition[0], Color);
		DrawBone(BonePosition[0], BonePosition[9], Color);
		DrawBone(BonePosition[9], BonePosition[5], Color);
		DrawBone(BonePosition[5], BonePosition[6], Color);
		DrawBone(BonePosition[9], BonePosition[4], Color);
		DrawBone(BonePosition[4], BonePosition[3], Color);
		DrawBone(BonePosition[7], BonePosition[8], Color);
		DrawBone(BonePosition[7], BonePosition[10], Color);
		DrawBone(BonePosition[10], BonePosition[2], Color);
		DrawBone(BonePosition[8], BonePosition[11], Color);
		DrawBone(BonePosition[11], BonePosition[1], Color);
	}

#pragma endregion
#pragma region utils
	inline BOOL SearchFiles(LPCTSTR lpszFileName, LPSEARCHFUNC lpSearchFunc, BOOL bInnerFolders = FALSE)
	{
		LPTSTR part;
		char tmp[MAX_PATH];
		char name[MAX_PATH];

		HANDLE hSearch = NULL;
		WIN32_FIND_DATA wfd;
		memset(&wfd, 0, sizeof(WIN32_FIND_DATA));

		if (bInnerFolders)
		{
			if (GetFullPathName(lpszFileName, MAX_PATH, tmp, &part) == 0) return FALSE;
			strcpy(name, part);
			strcpy(part, "*.*");
			wfd.dwFileAttributes = FILE_ATTRIBUTE_DIRECTORY;
			if (!((hSearch = FindFirstFile(tmp, &wfd)) == INVALID_HANDLE_VALUE))
				do
				{
					if (!strncmp(wfd.cFileName, ".", 1) || !strncmp(wfd.cFileName, "..", 2))
						continue;

					if (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
					{
						char next[MAX_PATH];
						if (GetFullPathName(lpszFileName, MAX_PATH, next, &part) == 0) return FALSE;
						strcpy(part, wfd.cFileName);
						strcat(next, "\\");
						strcat(next, name);

						SearchFiles(next, lpSearchFunc, TRUE);
					}
				} while (FindNextFile(hSearch, &wfd));
				FindClose(hSearch);
		}

		if ((hSearch = FindFirstFile(lpszFileName, &wfd)) == INVALID_HANDLE_VALUE)
			return TRUE;
		do
			if (!(wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
			{
				char file[MAX_PATH];
				if (GetFullPathName(lpszFileName, MAX_PATH, file, &part) == 0) return FALSE;
				strcpy(part, wfd.cFileName);

				lpSearchFunc(wfd.cFileName);
			}
		while (FindNextFile(hSearch, &wfd));
		FindClose(hSearch);
		return TRUE;
	}
	void ReadConfigs(LPCTSTR lpszFileName)
	{
		if (!strstr(lpszFileName, "gui.ini"))
		{
			ConfigList.push_back(lpszFileName);
		}
	}
	void RefreshConfigs()
	{
		ConfigList.clear();
		string ConfigDir = "C:\\BF1_HAX\\*.ini";
		CreateDirectoryW(L"C:\\BF1_HAX", NULL);
		SearchFiles(ConfigDir.c_str(), ReadConfigs, FALSE);
	}
	static D3DXVECTOR3 RotatePoint(D3DXVECTOR3 EntityPos, D3DXVECTOR3 LocalPlayerPos, int posX, int posY, int sizeX, int sizeY, float angle, float zoom, bool* viewCheck)
	{
		float zs = LocalPlayerPos.z - EntityPos.z;
		float xs = LocalPlayerPos.x - EntityPos.x;
		double Yaw = -(double)angle;
		float single = xs * (float)cos(Yaw) - zs * (float)sin(Yaw);
		float ypisilum1 = xs * (float)sin(Yaw) + zs * (float)cos(Yaw);
		*viewCheck = ypisilum1 < 0;
		single *= zoom;
		ypisilum1 *= zoom;
		single = single * 2.f;
		single = single + (float)(posX + sizeX / 2);
		ypisilum1 = ypisilum1 * 2.f;
		ypisilum1 = ypisilum1 + (float)(posY + sizeY / 2);
		if (single < (float)posX)
			single = (float)posX;
		if (ypisilum1 < (float)posY)
			ypisilum1 = (float)posY;
		if (single > (float)(posX + sizeX - 3))
			single = (float)(posX + sizeX - 3);
		if (ypisilum1 > (float)(posY + sizeY - 3))
			ypisilum1 = (float)(posY + sizeY - 3);


		return D3DXVECTOR3(single, ypisilum1, 0);
	}
	class MOUSE
	{
	private:
		// coordinate variable
		POINT point;
	public:
		//Mobile class function
		void Move(int x, int y);
		void RelativeMove(int cx, int cy);
		void SavePos();
		void RestorePos();
		//Lock enabled class
		void Lock();
		void Unlock();

	};

	// Move the mouse to the absolute position(X coordinate, Y coordinate)
	void MOUSE::Move(int x, int y)
	{
		this->point.x = x;
		this->point.y = y;
		::SetCursorPos(x, y);
	}

	// Move the mouse to the relative position(X displacement, Y displacement)
	void MOUSE::RelativeMove(int cx, int cy)
	{
		::ShowCursor(0);
		::GetCursorPos(&this->point);
		this->point.x += cx;
		this->point.y += cy;
		::SetCursorPos(this->point.x, this->point.y);
	}

	// Save the current location()
	void MOUSE::SavePos()
	{
		::GetCursorPos(&this->point);
	}

	// Restore mouse position()
	void MOUSE::RestorePos()
	{
		::SetCursorPos(this->point.x, this->point.y);
	}

	// Lock the mouse()
	void MOUSE::Lock()
	{
		POINT pt;
		RECT rt;
		::GetCursorPos(&pt);
		rt.left = rt.right = pt.x;
		rt.top = rt.bottom = pt.y;
		rt.right++;
		rt.bottom++;
		::ClipCursor(&rt);

	}

	//Unlock the mouse ()
	void MOUSE::Unlock()
	{
		::ClipCursor(NULL);
	}

	class Inputs {
	private:
		static DWORD SendScanCode(WORD scan, BOOL up)
		{
			INPUT inp = { 0 };
			inp.type = INPUT_KEYBOARD;
			inp.ki.wScan = scan;
			inp.ki.dwFlags = KEYEVENTF_SCANCODE | (up ? KEYEVENTF_KEYUP : 0);
			return SendInput(1, &inp, sizeof(inp)) ? NO_ERROR : GetLastError();
		}
	public:
		static bool mouse_move(int x, int y)
		{
			INPUT input;
			input.type = INPUT_MOUSE;
			input.mi.mouseData = 0;
			input.mi.time = 0;
			input.mi.dx = x;
			input.mi.dy = y;
			input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_VIRTUALDESK;
			SendInput(1, &input, sizeof(input));
			return true;
		}
		static DWORD SendVirtualKey(UINT vk, BOOL up)
		{
			UINT scan = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
			return scan ? SendScanCode(scan, up) : ERROR_NO_UNICODE_TRANSLATION;
		}
		bool static keydown(int key)
		{
			return (GetAsyncKeyState(key) & 0x8000) != 0;
		}
	};
	double GetCrossDistance(double x1, double y1, double x2, double y2)
	{
		return sqrt(pow((x2 - x1), 2) + pow((y2 - y1), 2));
	}
	void AimAtPosV2(float x, float y, float speed,float lock, bool smooth)
	{
		MOUSE move = MOUSE();
		int ScreenCenterX = Globals::rWidth / 2, ScreenCenterY = Globals::rHeight / 2;

		float AimSpeed = (float)speed;
		float TargetX = 0;
		float TargetY = 0;

		//X Axis
		if (x != 0)
		{
			if (x > ScreenCenterX)
			{
				TargetX = -(ScreenCenterX - x);
				TargetX /= AimSpeed;
				TargetX += lock;
				if (TargetX + ScreenCenterX > ScreenCenterX * 2) TargetX = 0;
			}

			if (x < ScreenCenterX)
			{
				TargetX = x - ScreenCenterX;
				TargetX /= AimSpeed;
				TargetX -= lock;
				if (TargetX + ScreenCenterX < 0) TargetX = 0;
			}
		}

		//Y Axis

		if (y != 0)
		{
			if (y > ScreenCenterY)
			{
				TargetY = -(ScreenCenterY - y);
				TargetY /= AimSpeed;
				TargetY += lock;
				if (TargetY + ScreenCenterY > ScreenCenterY * 2) TargetY = 0;
			}

			if (y < ScreenCenterY)
			{
				TargetY = y - ScreenCenterY;
				TargetY /= AimSpeed;
				TargetY -= lock;
				if (TargetY + ScreenCenterY < 0) TargetY = 0;
			}
		}

		if (!smooth)
		{
			//Inputs::mouse_move((int)TargetX, (int)(TargetY));
			move.RelativeMove((int)TargetX, (int)(TargetY));
			return;
		}

		TargetX /= 10;
		TargetY /= 10;

		if (abs(TargetX) < 1)
		{
			if (TargetX > 0)
			{
				TargetX = 1;
			}
			if (TargetX < 0)
			{
				TargetX = -1;
			}
		}
		if (abs(TargetY) < 1)
		{
			if (TargetY > 0)
			{
				TargetY = 1;
			}
			if (TargetY < 0)
			{
				TargetY = -1;
			}
		}
		//Inputs::mouse_move((int)TargetX, (int)(TargetY));
		move.RelativeMove((int)TargetX, (int)(TargetY));
	}
	D3DXVECTOR3 CalcSoldierFuturePos(D3DXVECTOR3 InVec)
	{
		D3DXVECTOR3 NewPos;
		if (WorldToScreen(InVec)) {
			NewPos.x = InVec.x;
			NewPos.y = InVec.y;
			NewPos.z = InVec.z;
		}
		else
		{
			NewPos.x = 0;
			NewPos.y = 0;
			NewPos.z = 0;
		}
		return NewPos;
	}
#pragma endregion
#pragma region menu utils
	static const char* sidebar_tabs[] = {
	"Aimbot",
	"Visual",
	"Misc",
	"Radar",
	"Colors",
	"Settings",
	"",
	};
	void DrawRectRainbow(int x, int y, int width, int height, float flSpeed, float& flRainbow)
	{
		ImDrawList* windowDrawList = ImGui::GetWindowDrawList();

		Color::Color colColor(0, 0, 0, 255);
		flRainbow += flSpeed;
		if (flRainbow > 1.f) flRainbow = 0.f;
		for (int i = 0; i < width; i++)
		{
			float hue = (1.f / (float)width) * i;
			hue -= flRainbow;
			if (hue < 0.f) hue += 1.f;

			Color::Color colRainbow = colColor.FromHSB(hue, 1.f, 1.f);
			windowDrawList->AddRectFilled(ImVec2(x + i, y), ImVec2(width, height), colRainbow.GetU32());
		}
	}

	template<size_t N>
	void render_tabs(const char* (&names)[N], int& activetab, float w, float h, bool sameline)
	{
		bool values[N] = { false };

		values[activetab] = true;

		for (auto i = 0u; i < N; ++i) {
			if (ImGui::Button(names[i], ImVec2{ w, h })) {
				activetab = i;
			}
			if (sameline && i < N - 1)
				ImGui::SameLine();
		}
	}
	int get_fps()
	{
		using namespace std::chrono;
		static int count = 0;
		static auto last = high_resolution_clock::now();
		auto now = high_resolution_clock::now();
		static int fps = 0;

		count++;

		if (duration_cast<milliseconds>(now - last).count() > 1000) {
			fps = count;
			count = 0;
			last = now;
		}

		return fps;
	}
	constexpr static float get_sidebar_item_width() { return 150.0f; }
	constexpr static float get_sidebar_item_height() { return  50.0f; }
	ImVec2 get_sidebar_size()
	{
		constexpr float padding = 10.0f;
		constexpr auto size_w = padding * 2.0f + get_sidebar_item_width();
		constexpr auto size_h = padding * 2.0f + (sizeof(sidebar_tabs) / sizeof(char*)) * get_sidebar_item_height();

		return ImVec2{ size_w, size_h };
	}
	const char* HitBoxesToScan[] =
	{
		"Head",
		"Neck",
		"Spine",
	};
	const char* EspBoxTtpe[] =
	{
		"3D",
		"Box",
		"Corners",
	};
	const char* HealthBarType[] =
	{
		"Horizontal Bar",
		"Vertical Bar",
	};
	const char* SkeletonType[] =
	{
		"Only Visible",
		"None",
	};
	const char* EspvehicleType[] =
	{
		"Esp DMG Color",
		"Esp Only",
	};
	const char* CrosshairType[] =
	{
		"Dot",
		"Plus",
		"Cross",
		"Gap",
		"Diagonal",
		"Swastika",
		"Off"
	};
	void aimbot()
	{
		ImGui::Checkbox(XorStr("Aimbot Micro"), &g_Options.AIMBOT.aimbot);
		ImGui::Separator();
		ImGui::BeginChild(XorStr("#Genral"), ImVec2(270, 175), true);
		{
			ImGui::Text(XorStr("Genral"));
			ImGui::Separator();
			ImGui::Hotkey(XorStr("AimKey"), &g_Options.AIMBOT.aimkey);
			ImGui::Checkbox(XorStr("AutoLock"), &g_Options.AIMBOT.aimbotauto);
			ImGui::SameLine();
			ImGui::Checkbox(XorStr("Draw Fov"), &g_Options.AIMBOT.drawfov);
			ImGui::SliderFloat(XorStr("FOV##1"), &g_Options.AIMBOT.FOV, 1.f, 180.0f, "%1.0");
			ImGui::SliderFloat(XorStr("Smooth"), &g_Options.AIMBOT.aimsmooth, 1.0f, 50.0f, "%1.0");
			ImGui::SliderFloat(XorStr("Lock"), &g_Options.AIMBOT.Lock, 0.0f, 3.0f, "%0.1");

		}ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild(XorStr("#Accuracy"), ImVec2(270, 175), true);
		{
			ImGui::Text(XorStr("Accuracy"));
			ImGui::Separator();
			ImGui::Checkbox(XorStr("AimBot Team"), &g_Options.AIMBOT.aimbotteam);
			ImGui::Checkbox(XorStr("AimBot Enemy"), &g_Options.AIMBOT.aimbotenemy);


		}ImGui::EndChild();
		ImGui::BeginChild(XorStr("#HitBox"), ImVec2(270, 150), true);
		{
			ImGui::Text(XorStr("HitBox"));
			ImGui::Separator();
			ImGui::Combo(XorStr("HitBox"), &g_Options.AIMBOT.hitboxbone, HitBoxesToScan, ARRAYSIZE(HitBoxesToScan));
			ImGui::Checkbox(XorStr("Draw Hitbox##1"), &g_Options.AIMBOT.drawhitbox);

		}ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild(XorStr("#angles"), ImVec2(270, 150), true);
		{
			ImGui::Text(XorStr("Angles"));
			ImGui::Separator();
			ImGui::SliderFloat(XorStr("XAxis"), &g_Options.AIMBOT.bPredict, 0.0f, 1.0f, "%0.1");
			ImGui::SliderFloat(XorStr("YAxis"), &g_Options.AIMBOT.bYAxis, 0.0f, 1.0f, "%0.1");

		}ImGui::EndChild();

	}
	void Visual()
	{
		ImGui::Checkbox(XorStr("Active"), &g_Options.ESP.EnableESP);
		ImGui::Separator();
		if (ImGui::Button(XorStr("ESP PLAYERS"), ImVec2(150, 0)))
		{
			g_Options.MENU.iVisualsSubTab = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button(XorStr("ESP VEHICLE"), ImVec2(150, 0)))
		{
			g_Options.MENU.iVisualsSubTab = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button(XorStr("FONT & SIZE"), ImVec2(150, 0)))
		{
			g_Options.MENU.iVisualsSubTab = 2;
		}
		ImGui::Separator();
		switch (g_Options.MENU.iVisualsSubTab)
		{
		case 0:
		{
			ImGui::BeginChild(XorStr("#ESP_TEAM"), ImVec2(270, 120), true);
			{
				ImGui::Text(XorStr("ESP TEAM"));
				ImGui::Separator();

				ImGui::Checkbox(XorStr("Box"), &g_Options.ESP.ESP_TEAM.teamBoxes);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Name"), &g_Options.ESP.ESP_TEAM.teamNames);
				ImGui::Checkbox(XorStr("HP"), &g_Options.ESP.ESP_TEAM.teamHealth);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Aimlines"), &g_Options.ESP.ESP_TEAM.teamSnaplines);
				ImGui::Checkbox(XorStr("Skeleton"), &g_Options.ESP.ESP_TEAM.teamSkeleton);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Fill-Box"), &g_Options.ESP.ESP_TEAM.teamfilledbox);
				ImGui::Checkbox(XorStr("Distance"), &g_Options.ESP.ESP_TEAM.teamDistance);
			}ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild(XorStr("#ESP_ENEMY"), ImVec2(270, 120), true);
			{
				ImGui::Text(XorStr("ESP ENEMY"));
				ImGui::Separator();
				ImGui::Checkbox(XorStr("Box"), &g_Options.ESP.ESP_ENEMY.enemyBoxes);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Name"), &g_Options.ESP.ESP_ENEMY.enemyNames);
				ImGui::Checkbox(XorStr("HP"), &g_Options.ESP.ESP_ENEMY.enemyHealth);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Aimlines"), &g_Options.ESP.ESP_ENEMY.enemySnaplines);
				ImGui::Checkbox(XorStr("Skeleton"), &g_Options.ESP.ESP_ENEMY.enemySkeleton);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Fill-Box"), &g_Options.ESP.ESP_ENEMY.enemyfilledbox);
				ImGui::Checkbox(XorStr("Distance"), &g_Options.ESP.ESP_ENEMY.enemyDistance);
			}ImGui::EndChild();
			ImGui::BeginChild(XorStr("#ESP_OPTIONS"), ImVec2(270, 180), true);
			{
				ImGui::Text(XorStr("ESP SETTINGS"));
				ImGui::Separator();
				ImGui::Combo(XorStr("Box"), &g_Options.ESP.EspBoxType, EspBoxTtpe, ARRAYSIZE(EspBoxTtpe));
				ImGui::Combo(XorStr("Health "), &g_Options.ESP.EspHealthBarType, HealthBarType, ARRAYSIZE(HealthBarType));
				ImGui::Combo(XorStr("Skeleton"), &g_Options.ESP.visSkeletontype, SkeletonType, ARRAYSIZE(SkeletonType));
				ImGui::SliderInt(XorStr("Distance##1"), &g_Options.ESP.distanceToPlayer, 1, 700, "%0");
				ImGui::SliderFloat(XorStr("Filled Box Alpha"), &g_Options.ESP.filledboxAlpha, 0.0f, 1.f, "%0");
				ImGui::Checkbox(XorStr("OutLine"), &g_Options.ESP.esp_Outline);

			}ImGui::EndChild();			
			break;
		}
		case 1:
		{
			ImGui::BeginChild(XorStr("#VEHICLE_TEAM"), ImVec2(270, 140), true);
			{
				ImGui::Text(XorStr("VEHICLE TEAM"));
				ImGui::Separator();
				ImGui::Checkbox(XorStr("Box"), &g_Options.ESP.VEHICLE_TEAM.teamVehicle);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Name"), &g_Options.ESP.VEHICLE_TEAM.teamVehicleName);
				ImGui::Checkbox(XorStr("DMG"), &g_Options.ESP.VEHICLE_TEAM.teamVehicledmg);
			}ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild(XorStr("#VEHICLE_ENEMY"), ImVec2(270, 140), true);
			{
				ImGui::Text(XorStr("VEHICLE ENEMY"));
				ImGui::Separator();
				ImGui::Checkbox(XorStr("Box"), &g_Options.ESP.VEHICLE_ENEMY.enemyVehicle);
				ImGui::SameLine(150);
				ImGui::Checkbox(XorStr("Name"), &g_Options.ESP.VEHICLE_ENEMY.enemyVehicleName);
				ImGui::Checkbox(XorStr("DMG"), &g_Options.ESP.VEHICLE_ENEMY.enemyVehicledmg);
			}ImGui::EndChild();
			ImGui::BeginChild(XorStr("#ESPVEHICLE_OPTIONS"), ImVec2(270, 140), true);
			{
				ImGui::Combo(XorStr("Esp Type"), &g_Options.ESP.ESPVehicleMode, EspvehicleType, ARRAYSIZE(EspvehicleType));
			}ImGui::EndChild();
			break;
		}
		case 2:
		{
			ImGui::Text(XorStr("ESP BOX Size"));
			ImGui::SliderFloat(XorStr("##BOX"), &g_Options.ESP.ESP_SETTINGS.espbox, 1, 20, "%0.1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP Line Size"));
			ImGui::SliderFloat(XorStr("##Lin"), &g_Options.ESP.ESP_SETTINGS.espline, 1, 20, "%0.1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP Skeleton Size"));
			ImGui::SliderFloat(XorStr("##Skeleton"), &g_Options.ESP.ESP_SETTINGS.espski, 1, 20, "%0.1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP Name Size"));
			ImGui::SliderInt(XorStr("##Name"), &g_Options.ESP.ESP_SETTINGS.espname, 1, 20, "%1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP Distance Size"));
			ImGui::SliderInt(XorStr("##Distance"), &g_Options.ESP.ESP_SETTINGS.espdis, 1, 20, "%1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP ID Size"));
			ImGui::SliderInt(XorStr("##ID"), &g_Options.ESP.ESP_SETTINGS.espid, 1, 20, "%1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP Vehicle Name Size"));
			ImGui::SliderInt(XorStr("##VehicleN"), &g_Options.ESP.ESP_SETTINGS.espvinname, 1, 20, "%0.1");
			ImGui::Separator();
			ImGui::Text(XorStr("ESP Vehicle Box Size"));
			ImGui::SliderFloat(XorStr("##VehicleB"), &g_Options.ESP.ESP_SETTINGS.espvinbox, 1, 20, "%0.1");
			break;
		}
		default:
			break;
		}
	}
	void Misc()
	{
		ImGui::BeginChild(XorStr("#Crosshair"), ImVec2(270, 200), true);
		{
			ImGui::Text(XorStr("Crosshair Options"));
			ImGui::Separator();
			ImGui::Combo(XorStr("Type"), &g_Options.MISC.crosshair.crosshair, CrosshairType, ARRAYSIZE(CrosshairType));
			ImGui::SliderFloat(XorStr("Size"), &g_Options.MISC.crosshair.size, 0.f, 10.f);
			ImGui::SliderFloat(XorStr("Stroke"), &g_Options.MISC.crosshair.stroke, 0.f, 10.f);

		}ImGui::EndChild();
	}
	void Radar()
	{
		ImGui::BeginChild(XorStr("#RADAR"), ImVec2(270, 250), true);
		{
			ImGui::Text(XorStr("Radar Options"));
			ImGui::Separator();
			ImGui::Checkbox(XorStr("Radar Window"), &g_Options.MISC.RADAR.radarwindow);
			ImGui::SliderFloat(XorStr("Radar Zoom"), &g_Options.MISC.RADAR.radrzoom, 0.f, 4.f, "Zoom: %.2f");
			ImGui::SliderFloat(XorStr("Radar Alpha"), &g_Options.MISC.RADAR.radralpha, 0.f, 1.f, "Alpha: %.2f");
			ImGui::SliderInt(XorStr("Distance ##1"), &g_Options.MISC.RADAR.distanceToPlayer, 0, 700, "%0");
			ImGui::Separator();
			ImGui::Checkbox(XorStr("Radar Team"), &g_Options.MISC.RADAR.PLAYERS.Team);
			ImGui::SameLine(150);
			ImGui::Checkbox(XorStr("Radar Enemy"), &g_Options.MISC.RADAR.PLAYERS.Enemy);
			ImGui::Checkbox(XorStr("Team Vehicle"), &g_Options.MISC.RADAR.VEHICLE.TeamVehicle);
			ImGui::SameLine(150);
			ImGui::Checkbox(XorStr("Enemy Vehicle"), &g_Options.MISC.RADAR.VEHICLE.EnemyVehicle);

		}ImGui::EndChild();
	}
	void Colors()
	{
		if (ImGui::Button(XorStr("ESP Team"), ImVec2(100, 0)))
		{
			g_Options.MENU.iColorsSubTab = 0;
		}
		ImGui::SameLine();
		if (ImGui::Button(XorStr("ESP Enemy"), ImVec2(100, 0)))
		{
			g_Options.MENU.iColorsSubTab = 1;
		}
		ImGui::SameLine();
		if (ImGui::Button(XorStr("ESP Vehicle"), ImVec2(100, 0)))
		{
			g_Options.MENU.iColorsSubTab = 2;
		}
		ImGui::SameLine();
		if (ImGui::Button(XorStr("ESP Radar"), ImVec2(100, 0)))
		{
			g_Options.MENU.iColorsSubTab = 3;
		}
		ImGui::SameLine();
		if (ImGui::Button(XorStr("Other"), ImVec2(100, 0)))
		{
			g_Options.MENU.iColorsSubTab = 4;
		}
		ImGui::Separator();
		switch (g_Options.MENU.iColorsSubTab)
		{
		case 0:
		{
			auto& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
			static int selectedItem = 0;
			ImVec2 lastCursor = ImGui::GetCursorPos();
			ImGui::ListBoxHeader("##0", ImVec2(240, 330));
			for (int i = 0; i < Globals::ColorsForPicker1.size(); i++)
			{
				bool selected = i == selectedItem;
				if (ImGui::Selectable(Globals::ColorsForPicker1[i].Name, selected))
					selectedItem = i;
			}
			ImGui::ListBoxFooter();
			ImGui::SameLine(0.0f, 15.0f);
			ImGui::BeginChild(XorStr("#generalcolors"), ImVec2(285, 330), false, ImGuiWindowFlags_NoScrollbar);
			{
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
				ImGui::BeginChild(XorStr("#ColorsChild"), ImVec2(285, 125), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ImGui::Spacing();
					ImGui::Spacing();
					ColorP col = Globals::ColorsForPicker1[selectedItem];
					int r = (col.Ccolor[0] * 255.f);
					int g = (col.Ccolor[1] * 255.f);
					int b = (col.Ccolor[2] * 255.f);
					ImGui::PushItemWidth(250);
					ImGui::SliderInt(XorStr("##red"), &r, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("R");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##green"), &g, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("G");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##blue"), &b, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("B");
					ImGui::PopItemWidth();
					col.Ccolor[0] = r / 255.0f;
					col.Ccolor[1] = g / 255.0f;
					col.Ccolor[2] = b / 255.0f;
				}ImGui::EndChild();
				ImGui::BeginChild(XorStr("#colorpreview"), ImVec2(285, 201), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ColorP col = Globals::ColorsForPicker1[selectedItem];
					ImGui::ColorPicker(XorStr("##COLOR_PICKER"), 250, 185, col.Ccolor);
				}ImGui::EndChild();
			}ImGui::EndChild();
		}
		break;
		case 1:
		{
			auto& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
			static int selectedItem = 0;
			ImVec2 lastCursor = ImGui::GetCursorPos();
			ImGui::ListBoxHeader("##0", ImVec2(240, 330));
			for (int i = 0; i < Globals::ColorsForPicker2.size(); i++)
			{
				bool selected = i == selectedItem;
				if (ImGui::Selectable(Globals::ColorsForPicker2[i].Name, selected))
					selectedItem = i;
			}
			ImGui::ListBoxFooter();
			ImGui::SameLine(0.0f, 15.0f);
			ImGui::BeginChild(XorStr("#generalcolors"), ImVec2(285, 330), false, ImGuiWindowFlags_NoScrollbar);
			{
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
				ImGui::BeginChild(XorStr("#ColorsChild"), ImVec2(285, 125), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ImGui::Spacing();
					ImGui::Spacing();
					ColorP col = Globals::ColorsForPicker2[selectedItem];
					int r = (col.Ccolor[0] * 255.f);
					int g = (col.Ccolor[1] * 255.f);
					int b = (col.Ccolor[2] * 255.f);
					ImGui::PushItemWidth(250);
					ImGui::SliderInt(XorStr("##red"), &r, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("R");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##green"), &g, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("G");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##blue"), &b, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("B");
					ImGui::PopItemWidth();
					col.Ccolor[0] = r / 255.0f;
					col.Ccolor[1] = g / 255.0f;
					col.Ccolor[2] = b / 255.0f;
				}ImGui::EndChild();
				ImGui::BeginChild(XorStr("#colorpreview"), ImVec2(285, 201), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ColorP col = Globals::ColorsForPicker2[selectedItem];
					ImGui::ColorPicker(XorStr("##COLOR_PICKER"), 250, 185, col.Ccolor);
				}ImGui::EndChild();
			}ImGui::EndChild();
		}
		break;
		case 2:
		{
			auto& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
			static int selectedItem = 0;
			ImVec2 lastCursor = ImGui::GetCursorPos();
			ImGui::ListBoxHeader("##0", ImVec2(240, 330));
			for (int i = 0; i < Globals::ColorsForPicker3.size(); i++)
			{
				bool selected = i == selectedItem;
				if (ImGui::Selectable(Globals::ColorsForPicker3[i].Name, selected))
					selectedItem = i;
			}
			ImGui::ListBoxFooter();
			ImGui::SameLine(0.0f, 15.0f);
			ImGui::BeginChild(XorStr("#generalcolors"), ImVec2(285, 330), false, ImGuiWindowFlags_NoScrollbar);
			{
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
				ImGui::BeginChild(XorStr("#ColorsChild"), ImVec2(285, 125), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ImGui::Spacing();
					ImGui::Spacing();
					ColorP col = Globals::ColorsForPicker3[selectedItem];
					int r = (col.Ccolor[0] * 255.f);
					int g = (col.Ccolor[1] * 255.f);
					int b = (col.Ccolor[2] * 255.f);
					ImGui::PushItemWidth(250);
					ImGui::SliderInt(XorStr("##red"), &r, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("R");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##green"), &g, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("G");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##blue"), &b, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("B");
					ImGui::PopItemWidth();
					col.Ccolor[0] = r / 255.0f;
					col.Ccolor[1] = g / 255.0f;
					col.Ccolor[2] = b / 255.0f;
				}ImGui::EndChild();
				ImGui::BeginChild(XorStr("#colorpreview"), ImVec2(285, 201), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ColorP col = Globals::ColorsForPicker3[selectedItem];
					ImGui::ColorPicker(XorStr("##COLOR_PICKER"), 250, 185, col.Ccolor);
				}ImGui::EndChild();
			}ImGui::EndChild();
		}
		break;
		case 3:
		{
			auto& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
			static int selectedItem = 0;
			ImVec2 lastCursor = ImGui::GetCursorPos();
			ImGui::ListBoxHeader("##0", ImVec2(240, 330));
			for (int i = 0; i < Globals::ColorsForPicker4.size(); i++)
			{
				bool selected = i == selectedItem;
				if (ImGui::Selectable(Globals::ColorsForPicker4[i].Name, selected))
					selectedItem = i;
			}
			ImGui::ListBoxFooter();
			ImGui::SameLine(0.0f, 15.0f);
			ImGui::BeginChild(XorStr("#generalcolors"), ImVec2(285, 330), false, ImGuiWindowFlags_NoScrollbar);
			{
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
				ImGui::BeginChild(XorStr("#ColorsChild"), ImVec2(285, 125), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ImGui::Spacing();
					ImGui::Spacing();
					ColorP col = Globals::ColorsForPicker4[selectedItem];
					int r = (col.Ccolor[0] * 255.f);
					int g = (col.Ccolor[1] * 255.f);
					int b = (col.Ccolor[2] * 255.f);
					ImGui::PushItemWidth(250);
					ImGui::SliderInt(XorStr("##red"), &r, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("R");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##green"), &g, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("G");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##blue"), &b, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("B");
					ImGui::PopItemWidth();
					col.Ccolor[0] = r / 255.0f;
					col.Ccolor[1] = g / 255.0f;
					col.Ccolor[2] = b / 255.0f;
				}ImGui::EndChild();
				ImGui::BeginChild(XorStr("#colorpreview"), ImVec2(285, 201), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ColorP col = Globals::ColorsForPicker4[selectedItem];
					ImGui::ColorPicker(XorStr("##COLOR_PICKER"), 250, 185, col.Ccolor);
				}ImGui::EndChild();
			}ImGui::EndChild();
		}
		break;
		case 4:
		{
			auto& style = ImGui::GetStyle();
			style.Colors[ImGuiCol_ChildWindowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.00f);
			static int selectedItem = 0;
			ImVec2 lastCursor = ImGui::GetCursorPos();
			ImGui::ListBoxHeader("##0", ImVec2(240, 330));
			for (int i = 0; i < Globals::ColorsForPicker5.size(); i++)
			{
				bool selected = i == selectedItem;
				if (ImGui::Selectable(Globals::ColorsForPicker5[i].Name, selected))
					selectedItem = i;
			}
			ImGui::ListBoxFooter();
			ImGui::SameLine(0.0f, 15.0f);
			ImGui::BeginChild(XorStr("#generalcolors"), ImVec2(285, 330), false, ImGuiWindowFlags_NoScrollbar);
			{
				style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
				ImGui::BeginChild(XorStr("#ColorsChild"), ImVec2(285, 125), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ImGui::Spacing();
					ImGui::Spacing();
					ColorP col = Globals::ColorsForPicker5[selectedItem];
					int r = (col.Ccolor[0] * 255.f);
					int g = (col.Ccolor[1] * 255.f);
					int b = (col.Ccolor[2] * 255.f);
					ImGui::PushItemWidth(250);
					ImGui::SliderInt(XorStr("##red"), &r, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("R");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##green"), &g, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("G");
					ImGui::Spacing();
					ImGui::SliderInt(XorStr("##blue"), &b, 0, 255, "%.0f"); ImGui::SameLine(); ImGui::Text("B");
					ImGui::PopItemWidth();
					col.Ccolor[0] = r / 255.0f;
					col.Ccolor[1] = g / 255.0f;
					col.Ccolor[2] = b / 255.0f;
				}ImGui::EndChild();
				ImGui::BeginChild(XorStr("#colorpreview"), ImVec2(285, 201), true);
				{
					style.Colors[ImGuiCol_ChildWindowBg] = ImColor(10, 10, 10, 255);
					ColorP col = Globals::ColorsForPicker5[selectedItem];
					ImGui::ColorPicker(XorStr("##COLOR_PICKER"), 250, 185, col.Ccolor);
				}ImGui::EndChild();
			}ImGui::EndChild();
		}
		break;
		default:
			break;
		}
	
	}
	
	void Settings()
	{
		static int iConfigSelect = 0;
		static int iMenuSheme = 1;
		static char ConfigName[64] = { 0 };
		ImGui::BeginChild(XorStr("first child"), ImVec2(547, 120), true);
		{
			ImGui::ComboBoxArray(XorStr("Select Config"), &iConfigSelect, ConfigList);
			ImGui::Separator();
			if (ImGui::Button(XorStr("Refresh Config")))
			{
				RefreshConfigs();
			}	
			ImGui::SameLine();
			if (ImGui::Button(XorStr("Save Config")))
			{
				SaveSettings("C:\\BF1_HAX\\" + ConfigList[iConfigSelect]);
			}
			ImGui::SameLine();
			if (ImGui::Button(XorStr("Load Config")))
			{
				LoadSettings("C:\\BF1_HAX\\" + ConfigList[iConfigSelect]);
			}
			ImGui::Separator();
			ImGui::InputText(XorStr("Config Name"), ConfigName, 64);
			if (ImGui::Button(XorStr("Create  Config")))
			{
				string ConfigFileName = ConfigName;
						
				if (ConfigFileName.size() < 1)
				{
					ConfigFileName = "";
				}
				if (ConfigFileName != "") {
					SaveSettings("C:\\BF1_HAX\\" + ConfigFileName + ".ini");
					RefreshConfigs();
				}
			}

			ImGui::Separator();
		}ImGui::EndChild();

		ImGui::BeginChild(XorStr("#VSync"), ImVec2(180, 50), true);
		{
			ImGui::Checkbox(XorStr("VSync"), &g_Options.MENU.o_VSync);
		}ImGui::EndChild();
		ImGui::SameLine();
		ImGui::BeginChild(XorStr("#Style"), ImVec2(180, 50), true);
		{
			const char* ThemesList[] = { "None","Purple" , "DefaultSheme1" , "RedSheme" , "Darkblue" , "MidNightSheme" , "NightSheme" , "DunnoSheme" , "BlueSheme"  , "MidNight2" , "BlackSheme2" , "Green" , "Pink" , "Blue" , "Yellow" , "BlackSheme" };

			ImGui::Combo(XorStr("Style"), &g_Options.MENU.Theme, ThemesList, IM_ARRAYSIZE(ThemesList));
		}ImGui::EndChild();
	}
#pragma endregion
		
	fb::ClientPlayer* ClosestVisibleEnemy()
	{
		fb::ClientPlayer* bClosestVisibleEnemy = NULL;
		float closest = FLT_MAX;
		fb::ClientPlayer* pLocalPlayer = (fb::ClientPlayer*)GetLocalPlayer();
		if (ValidPointer(pLocalPlayer)) {
				for (int i = 0; i < 64; i++) {
					fb::ClientPlayer* pPlayer = (fb::ClientPlayer*)GetPlayerById(i);
					if (ValidPointer(pPlayer))
					{

								std::vector<fb::ClientPlayer*> Players;
								Players.push_back(pPlayer);
								fb::ClientSoldierEntity* pSoldier = pPlayer->clientSoldierEntity;
								if (ValidPointer(pSoldier)) {
										fb::HealthComponent* pHealthComponent = pSoldier->healthcomponent;
										if (ValidPointer(pHealthComponent)) {
											if (pHealthComponent->m_Health > 0)
											{
												if checkSetting(g_Options.AIMBOT.aimbotteam, g_Options.AIMBOT.aimbotenemy) {

													if (pSoldier->IsVisible() || pPlayer->InVehicle()) {
														for (size_t i = 0; i < Players.size(); i++) {
															D3DXVECTOR3 textPos = D3DXVECTOR3(pSoldier->location.x, pSoldier->location.y, pSoldier->location.z);
															D3DXVECTOR3 out = textPos;
															if (WorldToScreen(out)) {
																float crossdis = GetCrossDistance(out.x, out.y, Globals::rWidth / 2, Globals::rHeight / 2);
																if ((crossdis < closest)) {


																	bClosestVisibleEnemy = Players[i];

																	closest = crossdis;
																}
															}
														}
													}
												}
											}
										}
								}

					}
				}
			
		}
		return (fb::ClientPlayer*)bClosestVisibleEnemy;
	}	
	void Runaimbot()
	{
		fb::ClientPlayer* player = ClosestVisibleEnemy();
		if (ValidPointer(player)) {
			fb::ClientSoldierEntity* aSoldier = player->clientSoldierEntity;
			if (ValidPointer(aSoldier)) {
				if ((Inputs::keydown(g_Options.AIMBOT.aimkey) || g_Options.AIMBOT.aimbotauto)) {
					D3DXVECTOR3 BONE = D3DXVECTOR3();
					float YAxis = g_Options.AIMBOT.bYAxis - 0.5f;
					float XAxis = g_Options.AIMBOT.bPredict - 0.5f;					
					std::vector<int> HitBoxesToScan{};
					switch (g_Options.AIMBOT.hitboxbone)
					{
					case 0:
						HitBoxesToScan.push_back(fb::BONE_Head);
						break;
					case 1:
						HitBoxesToScan.push_back(fb::BONE_Neck);
						break;
					case 2:
						HitBoxesToScan.push_back(fb::BONE_Spine);
						break;
					default:
						break;
					}
					for (auto hitbox : HitBoxesToScan)
					{
						if (hitbox != -1)
						{

								if (aSoldier->GetBonePos(hitbox, &BONE)) {

									D3DXVECTOR3 Hitbox = CalcSoldierFuturePos(D3DXVECTOR3(BONE.x, BONE.y - YAxis, BONE.z - XAxis));
									if (GetCrossDistance(Hitbox.x, Hitbox.y, Globals::rWidth / 2, Globals::rHeight / 2) <= g_Options.AIMBOT.FOV * 8)
									{
										auto roundPos = D3DXVECTOR2((float)round(Hitbox.x), (float)round(Hitbox.y));
										AimAtPosV2(roundPos.x, roundPos.y,g_Options.AIMBOT.aimsmooth, g_Options.AIMBOT.Lock, false);
										if (g_Options.AIMBOT.drawhitbox) {
											Renderer::GetInstance()->RenderCircleFilled(ImVec2(roundPos.x, roundPos.y), 5, D3DXCOLOR(255, 0, 0, 255));
											Renderer::GetInstance()->RenderCircle(ImVec2(roundPos.x, roundPos.y), 5, D3DXCOLOR(255, 255, 0, 255), 3);
										}
									}

								}

						}
					}
				}
			}
		}
		if (g_Options.AIMBOT.drawfov)
			Renderer::GetInstance()->RenderCircle(ImVec2(Globals::rWidth / 2, Globals::rHeight / 2), g_Options.AIMBOT.FOV * 8, D3DXCOLOR(255, 255, 255, 255));

	}
	void Crosshair()
	{
		switch (g_Options.MISC.crosshair.crosshair)
		{
		case Renderer::CrosshairStyle::Dot:
			Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Dot, Globals::rWidth / 2, Globals::rHeight / 2, g_Options.MISC.crosshair.size, g_Options.MISC.crosshair.stroke, D3DXCOLOR(g_Options.COLOR.CrossHair[0], g_Options.COLOR.CrossHair[1], g_Options.COLOR.CrossHair[2], 255.f));
			break;
		case Renderer::CrosshairStyle::Plus:
			Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Plus, Globals::rWidth / 2, Globals::rHeight / 2, g_Options.MISC.crosshair.size, g_Options.MISC.crosshair.stroke, D3DXCOLOR(g_Options.COLOR.CrossHair[0], g_Options.COLOR.CrossHair[1], g_Options.COLOR.CrossHair[2], 255.f));
			break;
		case Renderer::CrosshairStyle::Cross:
			Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Cross, Globals::rWidth / 2, Globals::rHeight / 2, g_Options.MISC.crosshair.size, g_Options.MISC.crosshair.stroke, D3DXCOLOR(g_Options.COLOR.CrossHair[0], g_Options.COLOR.CrossHair[1], g_Options.COLOR.CrossHair[2], 255.f));
			break;
		case Renderer::CrosshairStyle::Gap:
			Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Gap, Globals::rWidth / 2, Globals::rHeight / 2, g_Options.MISC.crosshair.size, g_Options.MISC.crosshair.stroke, D3DXCOLOR(g_Options.COLOR.CrossHair[0], g_Options.COLOR.CrossHair[1], g_Options.COLOR.CrossHair[2], 255.f));
			break;
		case Renderer::CrosshairStyle::Diagonal:
			Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Diagonal, Globals::rWidth / 2, Globals::rHeight / 2, g_Options.MISC.crosshair.size, g_Options.MISC.crosshair.stroke, D3DXCOLOR(g_Options.COLOR.CrossHair[0], g_Options.COLOR.CrossHair[1], g_Options.COLOR.CrossHair[2], 255.f));
			break;			
		case Renderer::CrosshairStyle::Swastika:
			Renderer::GetInstance()->DrawCrosshair(Renderer::CrosshairStyle::Swastika, Globals::rWidth / 2, Globals::rHeight / 2, g_Options.MISC.crosshair.size, g_Options.MISC.crosshair.stroke, D3DXCOLOR(g_Options.COLOR.CrossHair[0], g_Options.COLOR.CrossHair[1], g_Options.COLOR.CrossHair[2], 255.f));
			break;
		case 6:
			break;
		default:
			break;
		}
	}
	void DrawVisuals() {
		
		fb::GameRenderer* pGameRenderer = fb::GameRenderer::GetInstance();
		if (ValidPointer(pGameRenderer))
		{
			s_ViewProj = pGameRenderer->renderView->viewProj;
			fb::ClientPlayer* pLocalPlayer = (fb::ClientPlayer*)GetLocalPlayer();
			if (ValidPointer(pLocalPlayer)) {
				fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->clientSoldierEntity;
				if (ValidPointer(pLocalSoldier)) {
					for (int i = 0; i < 64; i++) {
						fb::ClientPlayer* pPlayer = (fb::ClientPlayer*)GetPlayerById(i);
						if (ValidPointer(pPlayer))
						{
							fb::ClientSoldierEntity* pSoldier = pPlayer->clientSoldierEntity;
							if (ValidPointer(pSoldier)) {
								fb::HealthComponent* pHealthComponent = pSoldier->healthcomponent;
								if (ValidPointer(pHealthComponent)) {
									D3DXCOLOR Playerbox;
									D3DXCOLOR Playerline;
									D3DXCOLOR PlayerSkeleton;
									D3DXCOLOR PlayerFiledBox;
									D3DXCOLOR PlayerDistance;
									D3DXCOLOR PlayerName;
									//
									D3DXCOLOR teamColor = D3DXCOLOR(g_Options.COLOR.TeamColor[0], g_Options.COLOR.TeamColor[1], g_Options.COLOR.TeamColor[2], 255.f);
									D3DXCOLOR occludedEnemyColor = D3DXCOLOR(g_Options.COLOR.OccludedEnemyColor[0], g_Options.COLOR.OccludedEnemyColor[1], g_Options.COLOR.OccludedEnemyColor[2], 255.f);
									D3DXCOLOR UnVisibleEnemyColor = D3DXCOLOR(g_Options.COLOR.VisibleEnemyColor[0], g_Options.COLOR.VisibleEnemyColor[1], g_Options.COLOR.VisibleEnemyColor[2], 255.f);

									D3DXCOLOR TeamSkeleton = D3DXCOLOR(g_Options.COLOR.TeamSkeleton[0], g_Options.COLOR.TeamSkeleton[1], g_Options.COLOR.TeamSkeleton[2], 255.f);
									D3DXCOLOR VisibleSkeleton = D3DXCOLOR(g_Options.COLOR.VisSkeleton[0], g_Options.COLOR.VisSkeleton[1], g_Options.COLOR.VisSkeleton[2], 255.f);
									D3DXCOLOR UnVisibleSkeleton = D3DXCOLOR(g_Options.COLOR.UnVisSkeleton[0], g_Options.COLOR.UnVisSkeleton[1], g_Options.COLOR.UnVisSkeleton[2], 255.f);

									D3DXCOLOR LineTeam = D3DXCOLOR(g_Options.COLOR.TeamLineColor[0], g_Options.COLOR.TeamLineColor[1], g_Options.COLOR.TeamLineColor[2], 255.f);
									D3DXCOLOR VisibleLineColor = D3DXCOLOR(g_Options.COLOR.VisLineColor[0], g_Options.COLOR.VisLineColor[1], g_Options.COLOR.VisLineColor[2], 255.f);
									D3DXCOLOR UnVisLineColor = D3DXCOLOR(g_Options.COLOR.UnVisLineColor[0], g_Options.COLOR.UnVisLineColor[1], g_Options.COLOR.UnVisLineColor[2], 255.f);

									D3DXCOLOR FiledBoxTeam = D3DXCOLOR(g_Options.COLOR.TeamFiledboxColor[0], g_Options.COLOR.TeamFiledboxColor[1], g_Options.COLOR.TeamFiledboxColor[2], g_Options.ESP.filledboxAlpha);
									D3DXCOLOR VisibleFiledBoxColor = D3DXCOLOR(g_Options.COLOR.VisFiledboxColor[0], g_Options.COLOR.VisFiledboxColor[1], g_Options.COLOR.VisFiledboxColor[2], g_Options.ESP.filledboxAlpha);
									D3DXCOLOR UnVisibleFiledBoxColor = D3DXCOLOR(g_Options.COLOR.UnVisFiledboxColor[0], g_Options.COLOR.UnVisFiledboxColor[1], g_Options.COLOR.UnVisFiledboxColor[2], g_Options.ESP.filledboxAlpha);

									D3DXCOLOR TeamDistanceColor = D3DXCOLOR(g_Options.COLOR.TeamDistanceColor[0], g_Options.COLOR.TeamDistanceColor[1], g_Options.COLOR.TeamDistanceColor[2], 255.f);
									D3DXCOLOR VisibleDistanceColor = D3DXCOLOR(g_Options.COLOR.VisDistanceColor[0], g_Options.COLOR.VisDistanceColor[1], g_Options.COLOR.VisDistanceColor[2], 255.f);
									D3DXCOLOR UnVisibleDistanceColor = D3DXCOLOR(g_Options.COLOR.UnVisDistanceColor[0], g_Options.COLOR.UnVisDistanceColor[1], g_Options.COLOR.UnVisDistanceColor[2], 255.f);

									D3DXCOLOR TeamNameColor = D3DXCOLOR(g_Options.COLOR.TeamNameColor[0], g_Options.COLOR.TeamNameColor[1], g_Options.COLOR.TeamNameColor[2], 255.f);
									D3DXCOLOR VisibleNameColor = D3DXCOLOR(g_Options.COLOR.VisNameColor[0], g_Options.COLOR.VisNameColor[1], g_Options.COLOR.VisNameColor[2], 255.f);
									D3DXCOLOR UnVisibleNameColor = D3DXCOLOR(g_Options.COLOR.UnVisNameColor[0], g_Options.COLOR.UnVisNameColor[1], g_Options.COLOR.UnVisNameColor[2], 255.f);
									//////

									if (pHealthComponent->m_Health > 0) {
										if (pPlayer->teamId == pLocalPlayer->teamId) {
											Playerbox = teamColor;
											Playerline = LineTeam;
											PlayerSkeleton = TeamSkeleton;
											PlayerFiledBox = FiledBoxTeam;
											PlayerDistance = TeamDistanceColor;
											PlayerName = TeamNameColor;
										}
										else {
											if (pSoldier->IsVisible()) {
												Playerbox = occludedEnemyColor;
												Playerline = VisibleLineColor;
												PlayerSkeleton = VisibleSkeleton;
												PlayerFiledBox = VisibleFiledBoxColor;
												PlayerDistance = VisibleDistanceColor;
												PlayerName = VisibleNameColor;
											}
											else {
												Playerbox = UnVisibleEnemyColor;
												Playerline = UnVisLineColor;
												PlayerSkeleton = UnVisibleSkeleton;
												PlayerFiledBox = UnVisibleFiledBoxColor;
												PlayerDistance = UnVisibleDistanceColor;
												PlayerName = UnVisibleNameColor;
											}
										}

										D3DXVECTOR3 textPos = D3DXVECTOR3(pSoldier->location.x, pSoldier->location.y, pSoldier->location.z);
										D3DXVECTOR3 footLocation = textPos;
										D3DXVECTOR3 headLocation = footLocation;
										if (pSoldier->poseType == 0) headLocation.y += 1.6;
										if (pSoldier->poseType == 1) headLocation.y += 1;
										if (pSoldier->poseType == 2) headLocation.y += .5;
										if (WorldToScreen(footLocation) && WorldToScreen(headLocation)) {
											float w2sHeight = Distance3D(footLocation, headLocation);
											float w2sWidth = w2sHeight;
											float heightoffset = Distance3D(footLocation, headLocation);
											float H = w2sHeight;
											float W = H / 2;
											float X = headLocation.x - W / 2;
											std::vector< Text > texts;

											if (pSoldier->poseType == 0) w2sWidth /= 2;
											if (pSoldier->poseType == 1) w2sWidth /= 1.5;
											float distanceToPlayer = Distance3D(pLocalSoldier->location, pSoldier->location);

											if (distanceToPlayer > g_Options.ESP.distanceToPlayer)
												continue;

											int iHpAmY = 1;
											if checkSetting(g_Options.ESP.ESP_TEAM.teamBoxes, g_Options.ESP.ESP_ENEMY.enemyBoxes) {

												if (g_Options.ESP.EspBoxType == 0)
												{
													DrawAABB(pSoldier->GetAABB(), textPos, pSoldier->authorativeYaw, Playerbox, g_Options.ESP.ESP_SETTINGS.espbox);
												}
												else if (g_Options.ESP.EspBoxType == 1)
												{
													if (!g_Options.ESP.esp_Outline)
														Renderer::GetInstance()->DrawBox(X, headLocation.y, W, H, Playerbox,g_Options.ESP.ESP_SETTINGS.espbox);
													else if (g_Options.ESP.esp_Outline)
														Renderer::GetInstance()->DrawOutlineBox(X, headLocation.y, W, H, Playerbox, g_Options.ESP.ESP_SETTINGS.espbox);

												}
												else
												{
													if (!g_Options.ESP.esp_Outline)
														Renderer::GetInstance()->DrawCoalBox(X, headLocation.y, W, H, Playerbox, g_Options.ESP.ESP_SETTINGS.espbox);
													else if (g_Options.ESP.esp_Outline)
														Renderer::GetInstance()->DrawOutlineCoalBox(X, headLocation.y, W, H, Playerbox, g_Options.ESP.ESP_SETTINGS.espbox);

												}
											}
											if (checkSetting(g_Options.ESP.ESP_TEAM.teamSnaplines, g_Options.ESP.ESP_ENEMY.enemySnaplines)) {
												Renderer::GetInstance()->RenderLine(ImVec2(Globals::rWidth / 2, 0), ImVec2(headLocation.x, headLocation.y), Playerline);
											}
											if (checkSetting(g_Options.ESP.ESP_TEAM.teamNames, g_Options.ESP.ESP_ENEMY.enemyNames)) {
												char txt[1024];
												sprintf(txt, "%s", pPlayer->name);
												texts.push_back(Text(txt, 0, g_Options.ESP.ESP_SETTINGS.espname, PlayerName));
											}
											if (checkSetting(g_Options.ESP.ESP_TEAM.teamDistance, g_Options.ESP.ESP_ENEMY.enemyDistance)) {
												char txt[1024];
												sprintf(txt, "%0.f m", distanceToPlayer);
												texts.push_back(Text(txt, 1, g_Options.ESP.ESP_SETTINGS.espdis, PlayerDistance));
											}
											if (checkSetting(g_Options.ESP.ESP_TEAM.teamHealth, g_Options.ESP.ESP_ENEMY.enemyHealth)) {
												if (g_Options.ESP.EspHealthBarType == 0)
													Renderer::GetInstance()->HorizontalDrawHealth(X, headLocation.y, W, 3, pHealthComponent->m_Health, pHealthComponent->m_MaxHealth, D3DXCOLOR(.48f,.98f, 0.f,1.f), D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
												else  
													Renderer::GetInstance()->VerticalHealthBar(X-6, headLocation.y,3, H, pHealthComponent->m_Health, pHealthComponent->m_MaxHealth, D3DXCOLOR(.48f, .98f, 0.f, 1.f), D3DXCOLOR(1.f, 0.f, 0.f, 1.f));
											}
											if (checkSetting(g_Options.ESP.ESP_TEAM.teamSkeleton, g_Options.ESP.ESP_ENEMY.enemySkeleton)) {
												if (g_Options.ESP.visSkeletontype == 0)
												{
													if (pSoldier->IsVisible())
														DrawSkeletonESP(pSoldier, PlayerSkeleton);
												}
												else
												{
													DrawSkeletonESP(pSoldier, PlayerSkeleton);
												}
											}
											if (checkSetting(g_Options.ESP.ESP_TEAM.teamfilledbox, g_Options.ESP.ESP_ENEMY.enemyfilledbox)) {
												Renderer::GetInstance()->DrawBoxFilled((int)X, (int)headLocation.y, (int)W, (int)H, PlayerFiledBox);
											}

											int middle[3] = { footLocation.y,footLocation.y,  footLocation.y };
											int Top[3] = { headLocation.y,headLocation.y,  headLocation.y };
											for (auto text : texts)
											{
												int fontsize = text.size;
												float offset = (text.text.length() * fontsize) / 5;
												switch (text.side)
												{
												case 0:
													Renderer::GetInstance()->RenderText(font, (char*)text.text.c_str(), ImVec2(headLocation.x - offset, Top[1] - (heightoffset / 4) - fontsize), text.size, text.color);
													Top[1] -= fontsize;
													break;
												case 1:
													Renderer::GetInstance()->RenderText(font, (char*)text.text.c_str(), ImVec2(footLocation.x - offset, middle[1]), text.size, text.color);
													middle[1] += fontsize;
													break;
												case 2:
													Renderer::GetInstance()->RenderText(font, (char*)text.text.c_str(), ImVec2(headLocation.x + (heightoffset / 4) + fontsize, Top[1] + fontsize), text.size,text.color);
													Top[1] += fontsize;
													break;
												}

											}


										}
									}
								}
								if (pPlayer->InVehicle())
								{
									D3DXCOLOR VehicleColor = D3DXCOLOR(g_Options.COLOR.VehicleColor[0], g_Options.COLOR.VehicleColor[1], g_Options.COLOR.VehicleColor[2], 255.f);
									D3DXCOLOR VehicleNameColor = D3DXCOLOR(g_Options.COLOR.VehicleNameColor[0], g_Options.COLOR.VehicleNameColor[1], g_Options.COLOR.VehicleNameColor[2], 255.f);

									fb::ClientVehicleEntity* pVehicle = pPlayer->clientVehicleEntity;

									fb::HealthComponent* pVehicleGetHealthComponent = pVehicle->GetHealthComponent();
									if (ValidPointer(pVehicleGetHealthComponent)) {
										if (pVehicleGetHealthComponent->m_VehicleHealth > 0) {
											fb::VehicleEntityData* pVehicleData = pVehicle->GetEntityData();
											if (ValidPointer(pVehicleData))
											{
												if (ValidPointer(pVehicleData->m_namesid)) {
													fb::LinearTransform_AABB TransformAABB = {};
													pVehicle->GetTransformAABB(TransformAABB);
													D3DXVECTOR3 Position = D3DXVECTOR3(TransformAABB.m_Transform._41, TransformAABB.m_Transform._42, TransformAABB.m_Transform._43);
													D3DXVECTOR3 PositionEnd = Position;
													PositionEnd.y += 3;
													float distanceToPlayer = Distance3D(pLocalSoldier->location, pSoldier->location);
													if (checkSetting(g_Options.ESP.VEHICLE_TEAM.teamVehicle, g_Options.ESP.VEHICLE_ENEMY.enemyVehicle)) {
														if (g_Options.ESP.ESPVehicleMode == 0) {
															float perc = (pVehicleGetHealthComponent->m_VehicleHealth / pVehicleData->m_FrontMaxHealth);
															D3DXCOLOR HealthColor = D3DXCOLOR(1 - perc, perc, 0, 1);
															VinDrawAABB(TransformAABB.m_Box, TransformAABB.m_Transform, HealthColor, g_Options.ESP.ESP_SETTINGS.espvinbox);
														}
														else
														{
															VinDrawAABB(TransformAABB.m_Box, TransformAABB.m_Transform, VehicleColor, g_Options.ESP.ESP_SETTINGS.espvinbox);
														}
													}
													if (WorldToScreen(Position) && WorldToScreen(PositionEnd))
													{
														float w2sHeight = Distance3D(Position, PositionEnd);
														float w2sWidth = w2sHeight;
														float heightoffset = Distance3D(Position, PositionEnd);
														float H = w2sHeight;
														float W = H / 2;
														float X = PositionEnd.x - W / 2;
														string vname = pVehicleData->m_namesid;
														vname = vname.erase(0, 11);
														if (checkSetting(g_Options.ESP.VEHICLE_TEAM.teamVehicleName, g_Options.ESP.VEHICLE_ENEMY.enemyVehicleName)) {
															int fontsize = 12;
															float offset = (vname.length() * fontsize) / 5;
															char txt[1024];
															sprintf(txt, "%s", vname.c_str());
															Renderer::GetInstance()->RenderText(font, txt, ImVec2(PositionEnd.x - offset, PositionEnd.y - (heightoffset / 4) - fontsize), 8, VehicleNameColor);
														}
														if (checkSetting(g_Options.ESP.VEHICLE_TEAM.teamVehicledmg, g_Options.ESP.VEHICLE_ENEMY.enemyVehicledmg)) {
															Renderer::GetInstance()->HorizontalDrawHealth(X, PositionEnd.y - 10, W, 3, (int)pVehicleGetHealthComponent->m_VehicleHealth, (int)pVehicleData->m_FrontMaxHealth, D3DXCOLOR(0, 1, 0, 1), D3DXCOLOR(1, 0, 0, 1));
														}
													}
												}
											}
										}
									}
								}
							}


						}
					}
				}
			}
		}
	}
	void DrawRadars()
	{
		auto& style = ImGui::GetStyle();
		style.Alpha = 1.f;
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1, 0.1f, 1.f);
		ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiSetCond_Appearing);

		if (ImGui::Begin(XorStr("Radar"), &g_Options.MISC.RADAR.radarwindow, ImGuiWindowFlags_NoCollapse))
		{
			ImVec2 siz = ImGui::GetContentRegionAvail();
			ImVec2 pos = ImGui::GetCursorScreenPos();
			float RadarCenterX = pos.x + (siz.x / 2);
			float RadarCenterY = pos.y + (siz.y / 2);
			Renderer::GetInstance()->RenderRect(ImVec2(pos.x, pos.y), ImVec2(pos.x + siz.x, pos.y + siz.y), ImGui::GetColorU32(ImVec4(100, 100, 100, 255)), 0.0F, -1, 1.5f);
			Renderer::GetInstance()->RenderLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(pos.x, pos.y), ImGui::GetColorU32(ImVec4(100, 100, 100, 100)), 1.f);
			Renderer::GetInstance()->RenderLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(pos.x + siz.x, pos.y), ImGui::GetColorU32(ImVec4(100, 100, 100, 100)), 1.f);
			Renderer::GetInstance()->RenderLine(ImVec2(pos.x, RadarCenterY), ImVec2(pos.x + siz.x, RadarCenterY), ImGui::GetColorU32(ImVec4(100, 100, 100, 100)), 1.f);
			Renderer::GetInstance()->RenderLine(ImVec2(RadarCenterX, RadarCenterY), ImVec2(RadarCenterX, pos.y + siz.y), ImGui::GetColorU32(ImVec4(100, 100, 100, 100)), 1.f);
			Renderer::GetInstance()->RenderCircleFilled(ImVec2(RadarCenterX, RadarCenterY), 2, ImGui::GetColorU32(ImVec4(100, 100, 100, 100)));
				fb::ClientPlayer* pLocalPlayer = (fb::ClientPlayer*)GetLocalPlayer();
				if (ValidPointer(pLocalPlayer)) {
					fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->clientSoldierEntity;
					if (ValidPointer(pLocalSoldier)) {
						for (int i = 0; i < 64; i++) {
							fb::ClientPlayer* pPlayer = (fb::ClientPlayer*)GetPlayerById(i);
							if (ValidPointer(pPlayer)) {
								fb::ClientSoldierEntity* pSoldier = pPlayer->clientSoldierEntity;
								if (ValidPointer(pSoldier)) {
									fb::HealthComponent* pHealthComponent = pSoldier->healthcomponent;
									if (ValidPointer(pHealthComponent)) {
										if (pHealthComponent->m_Health > 0) {
											D3DXCOLOR RadarPlayer;
											D3DXCOLOR RadarTeam = D3DXCOLOR(g_Options.COLOR.RadarTeam[0], g_Options.COLOR.RadarTeam[1], g_Options.COLOR.RadarTeam[2], 255);
											D3DXCOLOR VisRadarEnemy = D3DXCOLOR(g_Options.COLOR.VisRadarEnemy[0], g_Options.COLOR.VisRadarEnemy[1], g_Options.COLOR.VisRadarEnemy[2], 255);
											D3DXCOLOR UnVisRadarEnemy = D3DXCOLOR(g_Options.COLOR.UnVisRadarEnemy[0], g_Options.COLOR.UnVisRadarEnemy[1], g_Options.COLOR.UnVisRadarEnemy[2], 255);
											if (pPlayer->teamId == pLocalPlayer->teamId) {
												RadarPlayer = RadarTeam;
											}
											else {
												if (pSoldier->IsVisible()) {
													RadarPlayer = VisRadarEnemy;
												}
												else {
													RadarPlayer = UnVisRadarEnemy;
												}
											}
											float distanceToPlayer = Distance3D(pLocalSoldier->location, pSoldier->location);
											if (distanceToPlayer > g_Options.MISC.RADAR.distanceToPlayer)
												continue;
											if (checkSetting(g_Options.MISC.RADAR.PLAYERS.Team, g_Options.MISC.RADAR.PLAYERS.Enemy)) {

												bool viewCheck = false;
												D3DXVECTOR3 EntityPos = RotatePoint(pSoldier->location, pLocalSoldier->location, pos.x, pos.y, siz.x, siz.y, pLocalSoldier->authorativeYaw, g_Options.MISC.RADAR.radrzoom, &viewCheck);
												int s = 5;
												Renderer::GetInstance()->RenderCircleFilled(ImVec2(EntityPos.x, EntityPos.y), s, D3DXCOLOR(RadarPlayer.r, RadarPlayer.g, RadarPlayer.b, RadarPlayer.a));
											}
										}
									}

									if (pPlayer->InVehicle())
									{
										fb::ClientVehicleEntity* pVehicle = pPlayer->clientVehicleEntity;
										if (ValidPointer(pVehicle)) {
											fb::HealthComponent* pVehicleGetHealthComponent = pVehicle->GetHealthComponent();
											if (ValidPointer(pVehicleGetHealthComponent)) {
												if (pVehicleGetHealthComponent->m_VehicleHealth > 0) {
													fb::VehicleEntityData* pVehicleData = pVehicle->GetEntityData();
													if (ValidPointer(pVehicleData))
													{
														if (ValidPointer(pVehicleData->m_namesid)) {
															fb::LinearTransform_AABB TransformAABB = {};
															pVehicle->GetTransformAABB(TransformAABB);
															D3DXVECTOR3 Position = D3DXVECTOR3(TransformAABB.m_Transform._41, TransformAABB.m_Transform._42, TransformAABB.m_Transform._43);
															if (checkSetting(g_Options.MISC.RADAR.VEHICLE.TeamVehicle, g_Options.MISC.RADAR.VEHICLE.EnemyVehicle)) {
																bool viewCheck = false;
																D3DXVECTOR3 EntityPos = RotatePoint(Position, pLocalSoldier->location, pos.x, pos.y, siz.x, siz.y, pLocalSoldier->authorativeYaw, g_Options.MISC.RADAR.radrzoom, &viewCheck);
																int s = 5;
																Renderer::GetInstance()->RenderCircleFilled(ImVec2(EntityPos.x, EntityPos.y), s, D3DXCOLOR(g_Options.COLOR.VehicleRadar[0], g_Options.COLOR.VehicleRadar[1], g_Options.COLOR.VehicleRadar[2], 255));
															}
														}
													}
												}
											}

										}
									}

								}
							}
							
							
						}
					}
				}
			
		}ImGui::End();

	}
	void DrawMenu() {
		//ImGui Frame
		auto& style = ImGui::GetStyle();
		style.Alpha = 1.f;
		style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.1, 0.1f, 1.f);
		ImGui::SetNextWindowPos(ImVec2(500, 500), ImGuiSetCond_Appearing);
		const auto sidebar_size = get_sidebar_size();
		static int active_sidebar_tab = 0;
		Renderer::GetInstance()->DrawBoxFilled(0, 0, Globals::rWidth, Globals::rHeight, D3DXCOLOR(0.f, 0.f, 0.f, 0.70f));
		if (ImGui::Begin(XorStr("Zakaria#1077"), &Globals::Opened, ImVec2(760, 430), 0.98f, ImGuiWindowFlags_ShowBorders | ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize))
		{
			static float flRainbow;
			float flSpeed = 0.0003f;
			int curWidth = 1;
			ImVec2 curPos = ImGui::GetCursorPos();
			ImVec2 curWindowPos = ImGui::GetWindowPos();
			curPos.x += curWindowPos.x;
			curPos.y += curWindowPos.y;
			DrawRectRainbow(curPos.x, curPos.y * -1, ImGui::GetWindowSize().x + Globals::rWidth, curPos.y * 1.0, flSpeed, flRainbow);
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2::Zero);
			{
				ImGui::BeginGroupBox(XorStr("##sidebar"), sidebar_size);
				{
					ImGui::GetCurrentWindow()->Flags &= ~ImGuiWindowFlags_ShowBorders;

					render_tabs(sidebar_tabs, active_sidebar_tab, get_sidebar_item_width(), get_sidebar_item_height(), false);
				}
				ImGui::EndGroupBox();
			}
			ImGui::PopStyleVar();
			ImGui::SameLine();
			auto size = ImVec2{ 0.0f, sidebar_size.y };
			ImGui::BeginGroupBox(XorStr("##body"), size);
			if (active_sidebar_tab == 0) {
				aimbot();
			}
			else if (active_sidebar_tab == 1) {
				Visual();
			}
			else if (active_sidebar_tab == 2) {
				Misc();
			}
			else if (active_sidebar_tab == 3) {
				Radar();
			}
			else if (active_sidebar_tab == 4) {
				Colors();
			}
			else if (active_sidebar_tab == 5) {
				Settings();
			}
			ImGui::EndGroupBox();
			ImGui::TextColored(ImVec4{ 0.0f, 0.5f, 0.0f, 1.0f }, "FPS: %03d", get_fps());
			ImGui::End();
		}

	}
}

