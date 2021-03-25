#include "atu.h"
#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include <d3d9.h>
#include <tchar.h>
#include "myutils.h"
#include <stdio.h>

// Data
static LPDIRECT3D9              g_pD3D = NULL;
static LPDIRECT3DDEVICE9        g_pd3dDevice = NULL;
static D3DPRESENT_PARAMETERS    g_d3dpp = {};

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

static void HelpMarker(const char* desc);

float toStatusbarScale(int x, int min, int max);
float toStatusbarScale(float x, float min, float max);


// Main code
int main(int, char**)
{
	// Create application window
	//ImGui_ImplWin32_EnableDpiAwareness();
	WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("ImGui Example"), NULL };
	::RegisterClassEx(&wc);
	HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Dear ImGui DirectX9 Example"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

	// Initialize Direct3D
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		::UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	// Show the window
	::ShowWindow(hwnd, SW_SHOWDEFAULT);
	::UpdateWindow(hwnd);

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	/*ImPlot::CreateContext();*/
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsClassic();

	// Setup Platform/Renderer backends
	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(g_pd3dDevice);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'docs/FONTS.md' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
	//io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);

	// Our state
	ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);


	static struct AtuRelays atu_relays = DEFAULT_AtuRelays;

	// Main loop
	MSG msg;
	ZeroMemory(&msg, sizeof(msg));
	while (msg.message != WM_QUIT)
	{
		// Poll and handle messages (inputs, window resize, etc.)
		// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
		// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
		// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
		// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
		if (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
		{
			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			continue;
		}

		// Start the Dear ImGui frame
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
		/*static bool show_demo_window = true;
		ImGui::ShowDemoWindow(&show_demo_window);*/

		static bool bTune = false;

		    // FREQUENCY window
		{
			ImGui::Begin("TRX");

			static int freq_button = 0;

			static bool bPaTune = false;

			ImGui::Checkbox("PA Tune", &bPaTune);
			ImGui::RadioButton("1.8", &freq_button, 0); ImGui::SameLine();
			ImGui::RadioButton("3.5", &freq_button, 1); ImGui::SameLine();
			ImGui::RadioButton("7  ", &freq_button, 2);
			ImGui::RadioButton("10 ", &freq_button, 3); ImGui::SameLine();
			ImGui::RadioButton("14 ", &freq_button, 4); ImGui::SameLine();
			ImGui::RadioButton("18 ", &freq_button, 5);
			ImGui::RadioButton("21 ", &freq_button, 6); ImGui::SameLine();
			ImGui::RadioButton("24 ", &freq_button, 7); ImGui::SameLine();
			ImGui::RadioButton("28 ", &freq_button, 8);

			ImGui::End();
		}

		// ANTENNAS window
		{
			ImGui::Begin("Antennas");

			ImGui::RadioButton("Ant 1", & (atu_relays.u8Antenna), 0);
			ImGui::RadioButton("Ant 2", & (atu_relays.u8Antenna), 1);
			ImGui::RadioButton("Ant 3", & (atu_relays.u8Antenna), 2);

			ImGui::End();
		}

		// CONTROLS window
		{
			ImGui::Begin("Tune Controls");

			
			ImGui::Checkbox("Tune", &bTune); 
			ImGui::SameLine();	

			ImGui::Button("AUTO");

			static U16 step = 1;
			static U16 min = 1, max = 50;
			ImGui::SliderScalar("Step", ImGuiDataType_U8, &step, &min, &max, "%u");


			//ImGui::InputScalar("C", ImGuiDataType_U16, &(atu_relays.u16C), &step, NULL, "%u", ImGuiInputTextFlags_CharsHexadecimal);
			float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
			ImGui::PushButtonRepeat(true);
			if (ImGui::ArrowButton("left##1", ImGuiDir_Left)) 
			{ 
				atu_relays.u16C-= step;
				if (atu_relays.u16C > C_MAX)
					atu_relays.u16C = C_MAX;
			}
			ImGui::SameLine(0.0f, spacing);
			if (ImGui::ArrowButton("right##1", ImGuiDir_Right)) 
			{ 
				atu_relays.u16C += step; 
				if (atu_relays.u16C > C_MAX)
					atu_relays.u16C = 0;
			}
			ImGui::SameLine();
			ImGui::Text("C");
			ImGui::SameLine(0.0f, ImGui::GetStyle().IndentSpacing);
			ImGui::PopButtonRepeat();

			ImGui::PushButtonRepeat(true);
			if (ImGui::ArrowButton("left##2", ImGuiDir_Left)) 
			{ 
				atu_relays.u16L -= step; 
				if (atu_relays.u16L > L_MAX)
					atu_relays.u16L = L_MAX;
			}
			ImGui::SameLine(0.0f, spacing);
			if (ImGui::ArrowButton("right##2", ImGuiDir_Right)) 
			{ 
				atu_relays.u16L += step; 
				if (atu_relays.u16L > L_MAX)
					atu_relays.u16L = 0;
			}
			ImGui::PopButtonRepeat();
			ImGui::SameLine();
			ImGui::Text("L");

			static U16 minC = 0, maxC = C_MAX;
			ImGui::SliderScalar("C", ImGuiDataType_U16, &(atu_relays.u16C), &minC, &maxC, "%u");
			

			static U16 minL = 0, maxL = L_MAX;
			ImGui::SliderScalar("L", ImGuiDataType_U16, &(atu_relays.u16L), &minL, &maxL, "%u");
			
			ImGui::Checkbox("high R", &(atu_relays.bC2));

			atu_set_tune(&bTune);
			atu_set_relays(atu_relays);

			ImGui::End();		
		}

		

		// DATA VISUALIZATION window
		{
			ImGui::Begin("DATA");
			
			static struct AtuData atu_data = DEFAULT_AtuData;

			atu_get_data(&atu_data);
			static char overlay[20];

			sprintf(overlay, "%0.2f", atu_data.fSwr);
			ImGui::ProgressBar(toStatusbarScale(atu_data.fSwr, 1.0f, (float)SWR_MAX), ImVec2(0.0f, 0.0f), overlay); ImGui::SameLine();
			ImGui::Text("SWR");

			sprintf(overlay, "%d", atu_data.u16Vf);
			ImGui::ProgressBar(toStatusbarScale(atu_data.u16Vf, 1, ADC_MAX), ImVec2(0.0f, 0.0f), overlay);  ImGui::SameLine();
			ImGui::Text("V_f");

			sprintf(overlay, "%d", atu_data.u16Vr);
			ImGui::ProgressBar(toStatusbarScale(atu_data.u16Vr, 1, ADC_MAX), ImVec2(0.0f, 0.0f), overlay);  ImGui::SameLine();
			ImGui::Text("V_r");

			sprintf(overlay, "%d", atu_data.u16Vant);
			ImGui::ProgressBar(toStatusbarScale(atu_data.u16Vant, 1, ADC_MAX), ImVec2(0.0f, 0.0f), overlay);  ImGui::SameLine();
			ImGui::Text("V_ant");

			sprintf(overlay, "%d", atu_data.u16NotUsed);
			ImGui::ProgressBar(toStatusbarScale(atu_data.u16Vf, 1, ADC_MAX), ImVec2(0.0f, 0.0f), overlay);  ImGui::SameLine();
			ImGui::Text("V_notused");


			ImGui::End();
		}


		// CONNECTION window
		{
			ImGui::Begin("Connection");
			ImGui::PushItemWidth(ImGui::GetFontSize() * -12); // Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
			
			static bool bConnected = false; 
			ImGui::Checkbox("Connect", &bConnected);
		        static char str0[50] = "Com7";
			ImGui::InputText("Comm port name", str0, IM_ARRAYSIZE(str0));
			
			if (bConnected)
			{
				bConnected = atu_connect(str0);
			}
			else
			{
				atu_disconnect();
			}
				
			ImGui::End();
		}

		
		
		// Rendering
		ImGui::EndFrame();
		g_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		g_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clear_color.x*clear_color.w*255.0f), (int)(clear_color.y*clear_color.w*255.0f), (int)(clear_color.z*clear_color.w*255.0f), (int)(clear_color.w*255.0f));
		g_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
		if (g_pd3dDevice->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			g_pd3dDevice->EndScene();
		}
		HRESULT result = g_pd3dDevice->Present(NULL, NULL, NULL, NULL);

		// Handle loss of D3D9 device
		if (result == D3DERR_DEVICELOST && g_pd3dDevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}

	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	/*ImPlot::DestroyContext();*/
	ImGui::DestroyContext();


	CleanupDeviceD3D();
	::DestroyWindow(hwnd);
	::UnregisterClass(wc.lpszClassName, wc.hInstance);

	return 0;
}


// Helper functions

// Linear transformation of X [min to max] to  the range of [0 to 1]
float toStatusbarScale(int x, int min, int max)
{
	float res;

	res = ((float)x - (float)min) / ((float)max - (float)min);

	return res;
}

// Linear transformation of X [min to max] to  the range of [0 to 1]
float toStatusbarScale(float x, float min, float max)
{
	float res;

	res = (x - min) / (max - min);

	return res;
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((g_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	// Create the D3DDevice
	ZeroMemory(&g_d3dpp, sizeof(g_d3dpp));
	g_d3dpp.Windowed = TRUE;
	g_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	g_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
	g_d3dpp.EnableAutoDepthStencil = TRUE;
	g_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
	//g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
	if (g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3dpp, &g_pd3dDevice) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
	if (g_pD3D) { g_pD3D->Release(); g_pD3D = NULL; }
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	HRESULT hr = g_pd3dDevice->Reset(&g_d3dpp);
	if (hr == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
		{
			g_d3dpp.BackBufferWidth = LOWORD(lParam);
			g_d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}