#ifndef RENDER_MANAGER
#define RENDER_MANAGER

#include "Common\d3dUtil.h"
#include <d3d10.h>
#include <d3d10effect.h>
#include <string>
#include <DxErr.h>
#include <memory>
#include "Common\Camera.h"
#include "GameObject.h"
#include "FileLoader.h"
#include "FW1FontWrapper\FW1FontWrapper.h"
#include "Common\Sky.h"
#include "RiftManager.h"

class FileLoader;

class RenderManager
{
	public:
		static RenderManager& getInstance()
		{
			static RenderManager instance;
			return instance;
		}

		void SetClientSize(int aWidth, int aHeight)
		{
			mClientWidth = aWidth;
			mClientHeight = aHeight;
		}

		void SetRiftMan(RiftManager* aRiftMan)
		{
			riftMan = aRiftMan;
		}

		int getNumLights()
		{
			return mPointLights.size();
		}

		float AspectRatio() const
		{
			return static_cast<float>(mClientWidth) / mClientHeight;
		}

		void setVSYNC(bool on)
		{
			if(on)
				vsync = 1;
			else
				vsync = 0;
		}

		void ToggleOculusEffect()
		{
			if (postProcessingFlags & OculusEffect)
				RemovePostProcessingEffect(OculusEffect);
			else
				AddPostProcessingEffect(OculusEffect);
		}
	
		bool hasOculusEffect()
		{
			if(postProcessingFlags & OculusEffect)
				return true;
			return false;
		}

		//Returns the index of the newly created light for crests to manage.
		int CreateLight(XMFLOAT4 ambientLight, XMFLOAT4 diffuseLight, XMFLOAT4 specularLight, float range, XMFLOAT3 pos, XMFLOAT3 attenuation)
		{
			//Check if we are at max lights. Add if we aren't
			if(mPointLights.size() < MAX_LIGHTS)
			{
				PointLight aPointLight;
				aPointLight.Ambient = ambientLight;
				aPointLight.Diffuse = diffuseLight;
				aPointLight.Specular = specularLight;
				aPointLight.Range = range;
				aPointLight.Position = pos;
				aPointLight.Att = attenuation;
				aPointLight.On = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
				mPointLights.push_back(PointLight(aPointLight));
			}
			else
				return -1;
			//Pass back the index of the new light
			return (mPointLights.size() - 1);
		}

		bool InitDirect3D(HWND aWindow)
		{
			// Create the device and device context.

			UINT createDeviceFlags = 0;
			#if defined(DEBUG) || defined(_DEBUG)  
				createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
			#endif

			D3D_FEATURE_LEVEL featureLevel;
			D3D_FEATURE_LEVEL featureLevels[] =
			{
				D3D_FEATURE_LEVEL_11_0,
				D3D_FEATURE_LEVEL_10_1,
				D3D_FEATURE_LEVEL_10_0,
			};

			HRESULT hr = D3D11CreateDevice(
					0,                 // default adapter
					md3dDriverType,
					0,                 // no software device
					createDeviceFlags, 
					featureLevels, ARRAYSIZE(featureLevels),              // default feature level array
					D3D11_SDK_VERSION,
					&md3dDevice,
					&featureLevel,
					&md3dImmediateContext);

			if( FAILED(hr) )
			{
				MessageBox(0, L"D3D11CreateDevice Failed.", 0, 0);
				return false;
			}

			if( featureLevel != D3D_FEATURE_LEVEL_11_0 && featureLevel != D3D_FEATURE_LEVEL_10_0 && featureLevel != D3D_FEATURE_LEVEL_10_1)
			{
				MessageBox(0, L"Direct3D Feature Level 11, 10, 10.1 unsupported, exiting.", 0, 0);
			}

			usingDX11 = (featureLevel == D3D_FEATURE_LEVEL_11_0);

			// Check 4X MSAA quality support for our back buffer format.
			// All Direct3D 11 capable devices support 4X MSAA for all render 
			// target formats, so we only need to check quality support.

			HR(md3dDevice->CheckMultisampleQualityLevels(
				DXGI_FORMAT_R8G8B8A8_UNORM, 4, &m4xMsaaQuality));
			assert( m4xMsaaQuality > 0 );

			// Fill out a DXGI_SWAP_CHAIN_DESC to describe our swap chain.

			DXGI_SWAP_CHAIN_DESC sd;
			sd.BufferDesc.Width  = mClientWidth;
			sd.BufferDesc.Height = mClientHeight;
			sd.BufferDesc.RefreshRate.Numerator = 60;
			sd.BufferDesc.RefreshRate.Denominator = 1;
			sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
			sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

			// Use 4X MSAA? 
			if( mEnable4xMsaa )
			{
				sd.SampleDesc.Count   = 4;
				sd.SampleDesc.Quality = m4xMsaaQuality-1;
			}
			// No MSAA
			else
			{
				sd.SampleDesc.Count   = 1;
				sd.SampleDesc.Quality = 0;
			}

			sd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
			sd.BufferCount  = 1;
			sd.OutputWindow = aWindow;
			sd.Windowed     = true;
			sd.SwapEffect   = DXGI_SWAP_EFFECT_DISCARD;
			sd.Flags        = 0;

			// To correctly create the swap chain, we must use the IDXGIFactory that was
			// used to create the device.  If we tried to use a different IDXGIFactory instance
			// (by calling CreateDXGIFactory), we get an error: "IDXGIFactory::CreateSwapChain: 
			// This function is being called with a device from a different IDXGIFactory."

			IDXGIDevice* dxgiDevice = 0;
			HR(md3dDevice->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice));
	      
			IDXGIAdapter* dxgiAdapter = 0;
			HR(dxgiDevice->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgiAdapter));

			IDXGIFactory* dxgiFactory = 0;
			HR(dxgiAdapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgiFactory));

			HR(dxgiFactory->CreateSwapChain(md3dDevice, &sd, &mSwapChain));

			ReleaseCOM(dxgiDevice);
			ReleaseCOM(dxgiAdapter);
			ReleaseCOM(dxgiFactory);

			// The remaining steps that need to be carried out for d3d creation
			// also need to be executed every time the window is resized.  So
			// just call the OnResize method here to avoid code duplication.
	
			OnResize();

			BuildRasterizerStates();  

			//Create Text stuffs
			FW1CreateFactory(FW1_VERSION, &pFW1Factory);
			pFW1Factory->CreateFontWrapper(md3dDevice, L"Arial", &pFontWrapper);
			
			sky = new Sky(md3dDevice, L"Textures/grasscube1024.dds", 5000.0f, usingDX11);
			return true;
		}

		void setFullScreen(bool fs)
		{
			mSwapChain->SetFullscreenState(fs, NULL);
		}

		int GetClientWidth() { return mClientWidth; }
		int GetClientHeight() { return mClientHeight; }

		void ClearTargetToColor() //XMVECTORF32 clearColor
		{
			// Pretty self-explanatory. Clears the screen, essentially.
			md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Back Buffer"], 
				reinterpret_cast<const float*>(&Colors::Silver));
		}

		void ClearTargetToColor(const float* clearColor) //XMVECTORF32 clearColor
		{
			// Pretty self-explanatory. Clears the screen, essentially.
			md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Back Buffer"], 
				clearColor);
		}

		void DrawString(const char *text, float fontSize, float xPos, float yPos, UINT32 textColor)
		{	
			std::wstring s = s2ws(text);
			const wchar_t *str = s.c_str();
			pFontWrapper->DrawString(	md3dImmediateContext,
										str,// String
										fontSize,// Font size
										xPos,// X position
										yPos,// Y position
										textColor,// Text color, 0xAaBbGgRr
										FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
									);
		}

		void DrawString(const char *text, float fontSize, float xPos, float yPos, const float* clearColor)
		{	
			std::wstring s = s2ws(text);
			const wchar_t *str = s.c_str();

			UINT32 textColor;// = clearColor[1] * 0x1000000 + clearColor[2] * 0x10000 + clearColor[3] * 0x100 + clearColor[4];
			//memcpy(textColor, clearColor, sizeof(float));
			//UINT32 textColor = {clearColor[0], clearColor[1], clearColor[2], clearColor[3]};

			pFontWrapper->DrawString(	md3dImmediateContext,
				str,// String
				fontSize,// Font size
				xPos,// X position
				yPos,// Y position
				textColor,
				FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
				);
		}

		//I really hope you don't have to do wstrings
		void DrawString(const WCHAR *text, float fontSize, float xPos, float yPos, UINT32 textColor)
		{	
			pFontWrapper->DrawString(	md3dImmediateContext,
				text,// String
				fontSize,// Font size
				xPos,// X position
				yPos,// Y position
				textColor,// Text color, 0xAaBbGgRr
				FW1_NOGEOMETRYSHADER | FW1_RESTORESTATE// Flags (for example FW1_RESTORESTATE to keep context states unchanged)
				);
		}

		void DrawMenuBackground()
		{
			md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Back Buffer"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
			md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Default"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
			md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Back Buffer"], depthStencilViewsMap["Default"]);
		
			md3dImmediateContext->IASetInputLayout(mInputLayout);
			md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Menu Background"]);
			md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

			// Need to set matrices to identity to make sure it draws a 1-to-1 ratio.
			const float* identity = reinterpret_cast<const float*>(&XMMatrixIdentity());
			TexTransform->SetMatrix(identity);
			mfxWorld->SetMatrix(identity);
			mfxWorldInvTranspose->SetMatrix(identity);
			mfxViewProj->SetMatrix(identity);

			DrawQuad("TexturePassThrough");
		}

		void EndDrawMenu()
		{	
			HR(mSwapChain->Present(vsync, 0));
		}

		// Loop through all the buffers, drawing each instance for the game objects.
		void DrawGameObjects(string aTechniqueKey)
		{
			D3DX11_TECHNIQUE_DESC techDesc;
			techniqueMap[aTechniqueKey]->GetDesc( &techDesc );

			UINT stride[2] = { sizeof(Vertex), sizeof(InstancedData) };
			UINT offset[2] = { 0, 0 };

			// This will be each mesh's vertex and instance buffer.
			ID3D11Buffer* vbs[2] = {nullptr, nullptr};
			
			map<string, BufferPair>::iterator bufferItr = bufferPairs.begin();
			while (bufferItr != bufferPairs.end())
			{
				// Only draw if there is data to draw!
				if(mInstancedDataMap[bufferItr->first].size() >= 1)
				{
					// Get the data from the GPU, put correct data inside a container and only draw that.
					D3D11_MAPPED_SUBRESOURCE mappedData; 
					md3dImmediateContext->Map(bufferItr->second.instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData);

					InstancedData* dataView = reinterpret_cast<InstancedData*>(mappedData.pData);
					UINT mVisibleObjectCount = 0;

					vector<InstancedData> instanceVector = mInstancedDataMap[bufferItr->first];
					const UINT instanceSize = instanceVector.size();

					// This actually sets the instance data to draw by filling up dataView.
					for(UINT i = 0; i < instanceSize; ++i)
					{
						// If check goes here - only add in if we can see it / at least is inside frustum.
						if(instanceVector[i].isRendered)
							dataView[mVisibleObjectCount++] = instanceVector[i];
					}

					md3dImmediateContext->Unmap(bufferItr->second.instanceBuffer, 0);

					vbs[0] = bufferItr->second.vertexBuffer;
					vbs[1] = bufferItr->second.instanceBuffer;

					for(UINT p = 0; p < techDesc.Passes; ++p)
					{
						md3dImmediateContext->IASetVertexBuffers(0, 2, vbs, stride, offset);
						md3dImmediateContext->IASetIndexBuffer(bufferItr->second.indexBuffer, DXGI_FORMAT_R32_UINT, 0);
					
						D3D11_BUFFER_DESC indexDesc;
						bufferItr->second.indexBuffer->GetDesc(&indexDesc);
						int indexSize = indexDesc.ByteWidth / sizeof(UINT);

						techniqueMap[aTechniqueKey]->GetPassByIndex(p)->Apply(0, md3dImmediateContext);

						// Draw mVisibleObjetCount number of instances, each having indexSize vertices.
						md3dImmediateContext->DrawIndexedInstanced(indexSize, mVisibleObjectCount, 0, 0, 0);
					}
				}
				bufferItr++;
			}
		}

		void DrawScene(Camera* aCamera, vector<GameObject*> gameObjects)
		{
			// Performance increasers - Try to limit calls to size, map accessors, etc.
			const unsigned int totalGameobjs = gameObjects.size();
			vector<InstancedData>* instanceVector;
			string currentKey = "";
			D3DX11_TECHNIQUE_DESC techDesc;

			// Update each instance's world matrix with the corresponding GameObect's world matrix.
			for (unsigned int i = 0; i < totalGameobjs; ++i)
			{
				GameObject* aGameObject = gameObjects[i];
				string bufferKey = aGameObject->GetMeshKey();

				// Switch the instance we're updating.
				if (currentKey != bufferKey)
				{
					instanceVector = &mInstancedDataMap[bufferKey];
					currentKey = bufferKey;
				}

				unsigned int instanceCount = instanceCounts[bufferKey]++; 

				(*instanceVector)[instanceCount].isRendered = aGameObject->isSeen();
				(*instanceVector)[instanceCount].World = aGameObject->GetWorldMatrix();
			}
			
			if (postProcessingFlags & WireframeEffect)
				ToggleWireframe(true);
			
			mfxDirLights->SetRawValue(&mDirLights[0], 0, sizeof(DirectionalLight) * mDirLights.size());
			if(mPointLights.size() > 0)
				mfxPointLights->SetRawValue(&mPointLights[0], 0, (mPointLights.size()) * sizeof(PointLight));
			
			float numLights = (float)mPointLights.size();
			mfxNumLights->SetRawValue(&numLights, 0, sizeof(float));
			TexTransform->SetMatrix(reinterpret_cast<const float*>(&mTexTransform));
			// Bind the render target view and depth/stencil view to the pipeline.
			
			//mfxTextureAtlasVar->SetResourceArray(&shaderResourceViewsMap["BasicAtlas"], 0, 1); // Set texture atlas once for now.
			if (totalTextureAtlas == 0)
			{
				mfxTextureAtlasVar->SetResourceArray(&shaderResourceViewsMap["BasicAtlas"], 0, 1); // Set texture atlas once for now.
				++totalTextureAtlas;
			}

			md3dImmediateContext->IASetInputLayout(mInputLayout);
			md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			
			if (postProcessingFlags & OculusEffect)
			{	
				md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Distortion Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
				md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Default Render Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
				
				RenderToEye(riftMan->getLeftEyeParams(), aCamera);

				md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Distortion Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
				mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["BasicAtlas"]);
				TexTransform->SetMatrix(reinterpret_cast<const float*>(&XMMatrixIdentity()));
				RenderToEye(riftMan->getRightEyeParams(), aCamera);
				TexTransform->SetMatrix(reinterpret_cast<const float*>(&XMMatrixIdentity()));
			}
			else
			{
				md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Default Render Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
				md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Default"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
				md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Default Render Texture"], depthStencilViewsMap["Default"]);

				md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
				mEyePosW = aCamera->GetPosition();
				mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
				mfxViewProj->SetMatrix(reinterpret_cast<const float*>(&aCamera->ViewProj())); // This is now the view matrix - The world matrix is passed in via the instance and then multiplied there.
				DrawGameObjects("LightsWithAtlas");
				sky->Draw(md3dImmediateContext, *aCamera);
				md3dImmediateContext->IASetInputLayout(mInputLayout);
				md3dImmediateContext->OMSetDepthStencilState(0, 0);
			}
			
			ToggleWireframe(false);
			
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
 
			const float* identity = reinterpret_cast<const float*>(&XMMatrixIdentity());
 
			texelWidth->SetFloat(1.0f / mScreenViewport.Width);
			texelHeight->SetFloat(1.0f / mScreenViewport.Height);

			mfxWorld->SetMatrix(identity);
			mfxWorldInvTranspose->SetMatrix(identity);
			mfxViewProj->SetMatrix(identity);
			TexTransform->SetMatrix(identity);
			md3dImmediateContext->IASetVertexBuffers(0, 1, &bufferPairs["Quad"].vertexBuffer, &stride, &offset);
			md3dImmediateContext->IASetIndexBuffer(bufferPairs["Quad"].indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			#pragma region Post Processing - Blur
			if (postProcessingFlags & BlurEffect)
			{
				md3dImmediateContext->RSSetViewports(1, &mHalfScreenViewport);
				md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Blur Input Texture"], depthStencilViewsMap["Blur"]);
				md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Blur Input Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
				md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Blur"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
				techniqueMap["TexturePassThrough"]->GetDesc(&techDesc);
				mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Default Render Texture"]);
				for(UINT p = 0; p < techDesc.Passes; ++p)
				{
					md3dImmediateContext->IASetVertexBuffers(0, 1, &bufferPairs["Quad"].vertexBuffer, &stride, &offset);
					md3dImmediateContext->IASetIndexBuffer(bufferPairs["Quad"].indexBuffer, DXGI_FORMAT_R32_UINT, 0);

					techniqueMap["TexturePassThrough"]->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
					md3dImmediateContext->DrawIndexed(6, 0, 0);
				}

				for (int blurIndex = 0; blurIndex < blurCount; ++blurIndex)
				{
					md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Blur Output Texture"], depthStencilViewsMap["Blur"]);
					md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Blur Output Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
					md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Blur"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
					mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Blur Input Texture"]);

					techniqueMap["HorzBlur"]->GetDesc(&techDesc);
					for(UINT p = 0; p < techDesc.Passes; ++p)
					{
						techniqueMap["HorzBlur"]->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
						md3dImmediateContext->DrawIndexed(6, 0, 0);
					}
					
					// Unbind resource.
					UnbindShaderResource(mfxDiffuseMapVar, "HorzBlur");

					md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Blur Input Texture"], depthStencilViewsMap["Blur"]);
					md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Blur Input Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
					md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Blur"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
					mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Blur Output Texture"]);
				
					techniqueMap["VertBlur"]->GetDesc(&techDesc);
					for(UINT p = 0; p < techDesc.Passes; ++p)
					{
						techniqueMap["VertBlur"]->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
						md3dImmediateContext->DrawIndexed(6, 0, 0);
					}

					// Unbind resource.
					UnbindShaderResource(mfxDiffuseMapVar, "VertBlur");
				}

				md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
				md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Default Render Texture"], depthStencilViewsMap["Default"]);
				md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Default Render Texture"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
				md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Default"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
				mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Blur Output Texture"]);
				techniqueMap["TexturePassThrough"]->GetDesc(&techDesc);
				for(UINT p = 0; p < techDesc.Passes; ++p)
				{
					md3dImmediateContext->IASetVertexBuffers(0, 1, &bufferPairs["Quad"].vertexBuffer, &stride, &offset);
					md3dImmediateContext->IASetIndexBuffer(bufferPairs["Quad"].indexBuffer, DXGI_FORMAT_R32_UINT, 0);

					techniqueMap["TexturePassThrough"]->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
					md3dImmediateContext->DrawIndexed(6, 0, 0);
				}
			}
			#pragma endregion

			mfxDiffuseMapVar->SetResource(nullptr);
			md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Back Buffer"], depthStencilViewsMap["Default"]);
		
			// Pretty self-explanatory. Clears the screen, essentially.
			md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Back Buffer"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
			md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Default"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
			
			mfxWorld->SetMatrix(identity);
			mfxWorldInvTranspose->SetMatrix(identity);
			mfxViewProj->SetMatrix(identity);
			TexTransform->SetMatrix(identity);

			if (postProcessingFlags & OculusEffect)
			{
				//DrawQuad("OculusTech");
				md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
				mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Default Render Texture"]);
				DrawQuad("TexturePassThrough");
			}
			else
			{
				md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
				mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Default Render Texture"]);
				DrawQuad("TexturePassThroughWithCursor");
			}

			HR(mSwapChain->Present(vsync, 0));
			
			// Set shader view to null to prevent warnings.
			mfxDiffuseMapVar->SetResource(NULL);
			techniqueMap["Blur"]->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
			instanceCounts.clear();
			mfxBlurColor->SetRawValue(&XMFLOAT4(0.0f, 0.0f, 0.0f, 0.0f), 0, sizeof(XMFLOAT4));
		}
		
		void DrawQuad(string aTech)
		{
			D3DX11_TECHNIQUE_DESC techDesc;
			techniqueMap[aTech]->GetDesc(&techDesc);
			
			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			
			for(UINT p = 0; p < techDesc.Passes; ++p)
			{
				md3dImmediateContext->IASetVertexBuffers(0, 1, &bufferPairs["Quad"].vertexBuffer, &stride, &offset);
				md3dImmediateContext->IASetIndexBuffer(bufferPairs["Quad"].indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				techniqueMap[aTech]->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
				md3dImmediateContext->DrawIndexed(6, 0, 0);
			}
		}

		// Build a vertex and index buffer for each mesh.
		void BuildBuffers()
		{
			map<string, MeshData>::const_iterator itr = MeshMaps::MESH_MAPS.begin();
			while (itr != MeshMaps::MESH_MAPS.end())
			{
				string currentKey = itr->first;

				D3D11_BUFFER_DESC vertexBufferDesc;
				D3D11_SUBRESOURCE_DATA vertexData;
				ID3D11Buffer* vertexBuffer;

				vertexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				vertexBufferDesc.ByteWidth = itr->second.vertices.size() * sizeof(Vertex);
				vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
				vertexBufferDesc.CPUAccessFlags = 0;
				vertexBufferDesc.MiscFlags = 0;
				vertexBufferDesc.StructureByteStride = 0;
				vertexData.pSysMem = &itr->second.vertices[0];
				HR(md3dDevice->CreateBuffer(&vertexBufferDesc, &vertexData, &vertexBuffer));

				D3D11_BUFFER_DESC indexBufferDesc;
				D3D11_SUBRESOURCE_DATA indexData;
				ID3D11Buffer* indexBuffer;

				indexBufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
				indexBufferDesc.ByteWidth = itr->second.indices.size() * sizeof(UINT);
				indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
				indexBufferDesc.CPUAccessFlags = 0;
				indexBufferDesc.MiscFlags = 0;
				indexBufferDesc.StructureByteStride = 0;
				indexData.pSysMem = &itr->second.indices[0];
				HR(md3dDevice->CreateBuffer(&indexBufferDesc, &indexData, &indexBuffer));
				
				bufferPairs[itr->first].vertexBuffer = vertexBuffer;
				bufferPairs[itr->first].indexBuffer = indexBuffer;
				itr++;
			}
		}

		// Build an instance buffer based on a set of gameObjects.
		void BuildInstancedBuffer(vector<GameObject*> gameObjects)
		{
			// Clear the map of the vector of instance data.
			mInstancedDataMap.clear();
			// Loop through all game objects, setting the world matrix appropriately for each instance.
			for (unsigned int i = 0; i < gameObjects.size(); i++)
			{
				GameObject* aObject = gameObjects[i];

				GameMaterial aGameMaterial = GAME_MATERIALS[aObject->GetMaterialKey()];
				string bufferKey = aObject->GetMeshKey();
				
				// Fill up the fields of the InstancedData and then push it into a vector of instacedData of the appropriate kind.
				InstancedData theData;
				theData.World = aObject->GetWorldMatrix();
				theData.SurfMaterial = SURFACE_MATERIALS[aGameMaterial.SurfaceKey];
				theData.AtlasC = diffuseAtlasCoordsMap[aGameMaterial.DiffuseKey];
				theData.GlowColor = aGameMaterial.GlowColor;
				theData.TexScale = aObject->GetTexScale();

				mInstancedDataMap[bufferKey].push_back(theData);
			}
				
			// Go through and create each buffer.
			std::map<string, BufferPair>::iterator itr = bufferPairs.begin();
			while (itr != bufferPairs.end())
			{
				// Only create instance buffer if there is data to draw!
				if(mInstancedDataMap[itr->first].size() >= 1)
				{
					D3D11_BUFFER_DESC vbd;
					vbd.Usage = D3D11_USAGE_DYNAMIC;
					vbd.ByteWidth = sizeof(InstancedData) * mInstancedDataMap[itr->first].size();
					vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
					vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
					vbd.MiscFlags = 0;
					vbd.StructureByteStride = 0;
					
					// Release previous Instance buffer if needed.
					if (itr->second.instanceBuffer)
						ReleaseCOM(itr->second.instanceBuffer);

					HR(md3dDevice->CreateBuffer(&vbd, 0, &itr->second.instanceBuffer));

					instanceCounts[itr->first] = 0;
				}
				itr++;
			}
		}

		void LoadFile(wstring fileName, string fileNameS) //, bool RHCoordSys
		{
			FileLoader loaderMan = FileLoader();
			ObjModel objModel;
			loaderMan.LoadFile(md3dDevice, fileName, fileNameS, objModel, gameMats, surfaceMats, textureMan, false, false, false);
			GAME_MATERIALS[gameMats[gameMats.size() - 1].Name] = gameMats[gameMats.size() - 1];
			SURFACE_MATERIALS[gameMats[gameMats.size() - 1].Name] = surfaceMats[surfaceMats.size() - 1];
			//mObjModels.push_back(objModel);
		}

		void PushGameMaterial(GameMaterial gameMat)
		{
			gameMats.push_back(gameMat);
		}

		//This gets called during PVGame's load content. We need reference to md3dDevice and the shader resource view type stuff.
		void LoadContent()
		{
			//HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/WoodCrate01.dds", 0, 0, &mDiffuseMapSRV, 0 ));
			//HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, L"Textures/Wall01.dds", 0, 0, &mDiffuseMapSRV, 0 ));
			//HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice,L"defaultspec.dds", 0, 0, &mSpecMapRV, 0 ));
		}
		
		void LoadTexture(const string& aKey, const string& aFileName, const string& aType)
		{
			// Create the resource and save in a map with the provided key. Later, we can use the type to store in different maps (bump mapping, normal mapping, etc).
			ID3D11ShaderResourceView* aShaderResourceView;

			// Filenames need to be LPCWSTR, so a special function is needed to do the conversion.
			HR(D3DX11CreateShaderResourceViewFromFile(md3dDevice, s2ws(aFileName).c_str(), 0, 0, &aShaderResourceView, 0 ));
			shaderResourceViewsMap[aKey] = aShaderResourceView;
		}

		// Each texture has a uv offset in an atlas. So, we store that offset to send it to the shader so it can corretly sample from the larger texture while still retaining a [0,1] uv coordinate.
		void LoadTextureAtlasCoord(const string& aKey, XMFLOAT2 aCoord)
		{
			diffuseAtlasCoordsMap[aKey] = aCoord;
		}

		void BuildFX()
		{
			std::ifstream fin("fx/HardwareInstancing.fxo", std::ios::binary);

			fin.seekg(0, std::ios_base::end);
			int size = (int)fin.tellg();
			fin.seekg(0, std::ios_base::beg);
			std::vector<char> compiledShader(size);

			fin.read(&compiledShader[0], size);
			fin.close();
	
			HR(D3DX11CreateEffectFromMemory(&compiledShader[0], size, 
				0, md3dDevice, &mFX));

			// Quick fix to support DX10, assuming we don't care about lower levels.
			if (usingDX11)
			{
				techniqueMap["OculusTech"]						= mFX->GetTechniqueByName("OculusTech");
				techniqueMap["LightsWithAtlas"]					= mFX->GetTechniqueByName("LightsWithAtlas");
				techniqueMap["LightsWithoutAtlas"]				= mFX->GetTechniqueByName("LightsWithoutAtlas");
				techniqueMap["TexturePassThrough"]				= mFX->GetTechniqueByName("TexturePassThrough");
				techniqueMap["HorzBlur"]						= mFX->GetTechniqueByName("HorzBlur");
				techniqueMap["VertBlur"]						= mFX->GetTechniqueByName("VertBlur");
				techniqueMap["Blur"]							= mFX->GetTechniqueByName("Blur");
				techniqueMap["TexturePassThroughWithCursor"]	= mFX->GetTechniqueByName("TexturePassThroughWithCursor");
			}
			else
			{
				techniqueMap["LightsWithAtlas"]					= mFX->GetTechniqueByName("LightsWithAtlasDX10");
				techniqueMap["LightsWithoutAtlas"]              = mFX->GetTechniqueByName("LightsWithoutAtlasDX10");
				techniqueMap["TexturePassThrough"]				= mFX->GetTechniqueByName("TexturePassThroughDX10");
				techniqueMap["HorzBlur"]						= mFX->GetTechniqueByName("HorzBlurDX10");
				techniqueMap["VertBlur"]						= mFX->GetTechniqueByName("VertBlurDX10");
				techniqueMap["Blur"]							= mFX->GetTechniqueByName("BlurDX10");
				techniqueMap["TexturePassThroughWithCursor"]	= mFX->GetTechniqueByName("TexturePassThroughWithCursorDX10");
				techniqueMap["OculusTech"]						= mFX->GetTechniqueByName("OculusTechDX10");
			}

			// Creates association between shader variables and program variables.
			mfxViewProj				= mFX->GetVariableByName("gViewProj")->AsMatrix();
			mfxOcView				= mFX->GetVariableByName("gOcView")->AsMatrix();
			mfxWorld				= mFX->GetVariableByName("gWorld")->AsMatrix();
			mfxWorldInvTranspose	= mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
			mfxEyePosW				= mFX->GetVariableByName("gEyePosW")->AsVector();
			mfxDirLights			= mFX->GetVariableByName("gDirLights");
			mfxPointLights			= mFX->GetVariableByName("testLights");
			mfxSpotLight			= mFX->GetVariableByName("gSpotLight");
			mfxMaterial				= mFX->GetVariableByName("gMaterial");
			mfxDiffuseMapVar		= mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
			mfxTextureAtlasVar		= mFX->GetVariableByName("environmentAtlas")->AsShaderResource();
			mfxSpecMapVar			= mFX->GetVariableByName("gSpecMap")->AsShaderResource();
			mfxNumLights			= mFX->GetVariableByName("numLights");
			mfxBlurColor			= mFX->GetVariableByName("gBlurColor")->AsVector();

			// Rift variables.
			mfxLensCenter			= mFX->GetVariableByName("LensCenter")->AsVector();
			mfxScreenCenter			= mFX->GetVariableByName("ScreenCenter")->AsVector();
			mfxScale				= mFX->GetVariableByName("Scale")->AsVector();
			mfxScaleIn				= mFX->GetVariableByName("ScaleIn")->AsVector();
			mfxHmdWarpParam			= mFX->GetVariableByName("HmdWarpParam")->AsVector();
			mfxChromAbParam			= mFX->GetVariableByName("ChromAbParam")->AsVector();
			mfxScreenSize			= mFX->GetVariableByName("gScreenSize")->AsVector();

			TexTransform			= mFX->GetVariableByName("gTexTransform")->AsMatrix();
			texelWidth				= mFX->GetVariableByName("gTexelWidth")->AsScalar();
			texelHeight				= mFX->GetVariableByName("gTexelHeight")->AsScalar();

			float clientSize[2] = {(float)mClientWidth, (float)mClientHeight};
			mfxScreenSize->SetRawValue(&clientSize, 0, 2 * sizeof(float));
		}

		void BuildVertexLayout()
		{
			// Create the vertex input layout.
			D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
				// FORMAT:
				// { Semantic name for shader, semantic index if semantic name is already used (world is 4x4 matrix or 4 4-element matrices, so it goes 0-3,
				//	 Format of data - Everything is floats and number of letters is how many elements it is, Input slot, byte offset between previous element, 
				//   whether the element is per vertex or per instance, and how many instances to draw with that data.

				// Basic Vertex elements.
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0},
				// These are instanced elements - note the 'per instance data' part. 
				{ "WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "MATERIAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "MATERIAL", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 80, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "MATERIAL", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 96, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "MATERIAL", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 112, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
				{ "GLOWCOLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 128, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{ "TEXSCALE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 144, D3D11_INPUT_PER_INSTANCE_DATA, 1},
				{ "ATLASCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 160, D3D11_INPUT_PER_INSTANCE_DATA, 1}
			};

			// Create the input layout
			D3DX11_PASS_DESC passDesc;
			techniqueMap.begin()->second->GetPassByIndex(0)->GetDesc(&passDesc);
			HR(md3dDevice->CreateInputLayout(vertexDesc, 14, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
		}

		void ToggleLight(int index)
		{
			if (index >= 0 && index < (int)mPointLights.size())
				mPointLights[index].On.x = (mPointLights[index].On.x == 0.0f) ? 1.0f : 0.0f;
		}

		void EnableLight(int index)
		{
			if (index >= 0 && index < (int)mPointLights.size())
				mPointLights[index].On.x = 1;
		}

		void DisableLight(int index)
		{
			if (index >= 0 && index < (int)mPointLights.size())
				mPointLights[index].On.x = 0;
		}
		
		void SetLightPosition(int index, btVector3* targetV3)
		{
			if (index >= 0 && index < (int)mPointLights.size())
				mPointLights[index].Position = XMFLOAT3(targetV3->x(), targetV3->y(), targetV3->z());
		}

		void ChangeBlurCount(int aValue)
		{
			blurCount += aValue;
			blurCount = max(0, blurCount);
			blurCount = min(blurCount, MAX_BLURS);
		}

		void AddPostProcessingEffect(PostProcessingEffects anEffect)
		{
			postProcessingFlags |= anEffect;
		}

		void RemovePostProcessingEffect(PostProcessingEffects anEffect)
		{
			postProcessingFlags &= (~anEffect);
		}

		/* //Tried to overload this. It wouldn't take.
		void SetLightPosition(int index, XMFLOAT4* targetV4)
		{
			mPointLights[index].Position = XMFLOAT3(targetV4->x, targetV4->y, targetV4->z);
		}*/

		btVector3 getLightPosition(int index)
		{
			return btVector3(mPointLights[index].Position.x, mPointLights[index].Position.y, mPointLights[index].Position.z); 
		}

		void OnResize()
		{
			assert(md3dImmediateContext);
			assert(md3dDevice);
			assert(mSwapChain);

			// Release the old views, as they hold references to the buffers we
			// will be destroying.  Also release the old depth/stencil buffer.

			ReleaseResizeMaps();

			// Resize the swap chain and recreate the render target view.
			HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
			ID3D11Texture2D* backBuffer;
			HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
			HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &renderTargetViewsMap["Back Buffer"]));
			ReleaseCOM(backBuffer);

			// Create the depth/stencil buffer and view.

			D3D11_TEXTURE2D_DESC depthStencilDesc;
	
			depthStencilDesc.Width     = mClientWidth;
			depthStencilDesc.Height    = mClientHeight;
			depthStencilDesc.MipLevels = 1;
			depthStencilDesc.ArraySize = 1;
			depthStencilDesc.Format    = DXGI_FORMAT_D24_UNORM_S8_UINT;

			// Use 4X MSAA? --must match swap chain MSAA values.
			if( mEnable4xMsaa )
			{
				depthStencilDesc.SampleDesc.Count   = 4;
				depthStencilDesc.SampleDesc.Quality = m4xMsaaQuality-1;
			}
			// No MSAA
			else
			{
				depthStencilDesc.SampleDesc.Count   = 1;
				depthStencilDesc.SampleDesc.Quality = 0;
			}

			depthStencilDesc.Usage          = D3D11_USAGE_DEFAULT;
			depthStencilDesc.BindFlags      = D3D11_BIND_DEPTH_STENCIL;
			depthStencilDesc.CPUAccessFlags = 0; 
			depthStencilDesc.MiscFlags      = 0;

			HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &texture2DMap["Default Depth Stencil Buffer Texture"]));
			HR(md3dDevice->CreateDepthStencilView(texture2DMap["Default Depth Stencil Buffer Texture"], 0, &depthStencilViewsMap["Default"]));
			HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &texture2DMap["Oculus Depth Stencil Buffer Texture"]));
			HR(md3dDevice->CreateDepthStencilView(texture2DMap["Oculus Depth Stencil Buffer Texture"], 0, &depthStencilViewsMap["Oculus"]));

			depthStencilDesc.Width /= 2;
			depthStencilDesc.Height /= 2;
			HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &texture2DMap["Blur Depth Stencil Buffer Texture"]));
			HR(md3dDevice->CreateDepthStencilView(texture2DMap["Blur Depth Stencil Buffer Texture"], 0, &depthStencilViewsMap["Blur"]));

			int ocTexW = (int)ceil(riftMan->getStereo().GetDistortionScale() * mClientWidth),
				ocTexH = (int)ceil(riftMan->getStereo().GetDistortionScale() * mClientHeight);

			depthStencilDesc.Width = ocTexW;
			depthStencilDesc.Height = ocTexH;

			HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &texture2DMap["Distortion Depth Stencil Buffer Texture"]));
			HR(md3dDevice->CreateDepthStencilView(texture2DMap["Distortion Depth Stencil Buffer Texture"], 0, &depthStencilViewsMap["Distortion"]));

			// Bind the render target view and depth/stencil view to the pipeline.
			md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Back Buffer"], depthStencilViewsMap["Default"]);
	
			// Create a Texture2D we can render to.
			// Create the depth/stencil buffer and view.

			D3D11_TEXTURE2D_DESC renderTextureDesc;

			renderTextureDesc.Width     = mClientWidth;
			renderTextureDesc.Height    = mClientHeight;
			renderTextureDesc.MipLevels = 1;
			renderTextureDesc.ArraySize = 1;
			renderTextureDesc.Format    = DXGI_FORMAT_R8G8B8A8_UNORM;

			// Use 4X MSAA? --must match swap chain MSAA values.
			if( mEnable4xMsaa )
			{
				renderTextureDesc.SampleDesc.Count   = 4;
				renderTextureDesc.SampleDesc.Quality = m4xMsaaQuality-1;
			}
			// No MSAA
			else
			{
				renderTextureDesc.SampleDesc.Count   = 1;
				renderTextureDesc.SampleDesc.Quality = 0;
			}

			renderTextureDesc.Usage          = D3D11_USAGE_DEFAULT;
			renderTextureDesc.BindFlags      = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			renderTextureDesc.CPUAccessFlags = 0; 
			renderTextureDesc.MiscFlags      = 0;

			HR(md3dDevice->CreateTexture2D(&renderTextureDesc, 0, &texture2DMap["Default Render Texture"]));
			HR(md3dDevice->CreateRenderTargetView(texture2DMap["Default Render Texture"], 0, &renderTargetViewsMap["Default Render Texture"]));
			HR(md3dDevice->CreateTexture2D(&renderTextureDesc, 0, &texture2DMap["Oculus Texture"]));
			HR(md3dDevice->CreateRenderTargetView(texture2DMap["Oculus Texture"], 0, &renderTargetViewsMap["Oculus Texture"]));

			renderTextureDesc.Width /= 2;	
			renderTextureDesc.Height /= 2;
			HR(md3dDevice->CreateTexture2D(&renderTextureDesc, 0, &texture2DMap["Blur Input Texture"]));
			HR(md3dDevice->CreateRenderTargetView(texture2DMap["Blur Input Texture"], 0, &renderTargetViewsMap["Blur Input Texture"]));
			HR(md3dDevice->CreateTexture2D(&renderTextureDesc, 0, &texture2DMap["Blur Output Texture"]));
			HR(md3dDevice->CreateRenderTargetView(texture2DMap["Blur Output Texture"], 0, &renderTargetViewsMap["Blur Output Texture"]));

			renderTextureDesc.Width = ocTexW;	
			renderTextureDesc.Height = ocTexH;
			HR(md3dDevice->CreateTexture2D(&renderTextureDesc, 0, &texture2DMap["Distortion Texture"]));
			HR(md3dDevice->CreateRenderTargetView(texture2DMap["Distortion Texture"], 0, &renderTargetViewsMap["Distortion Texture"]));

			D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
			srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;

			HR(md3dDevice->CreateShaderResourceView(texture2DMap["Default Render Texture"], &srvDesc, &shaderResourceViewsMap["Default Render Texture"]));
			HR(md3dDevice->CreateShaderResourceView(texture2DMap["Blur Output Texture"], &srvDesc, &shaderResourceViewsMap["Blur Output Texture"]));
			HR(md3dDevice->CreateShaderResourceView(texture2DMap["Blur Input Texture"], &srvDesc, &shaderResourceViewsMap["Blur Input Texture"]));
			HR(md3dDevice->CreateShaderResourceView(texture2DMap["Oculus Texture"], &srvDesc, &shaderResourceViewsMap["Oculus Texture"]));
			HR(md3dDevice->CreateShaderResourceView(texture2DMap["Distortion Texture"], &srvDesc, &shaderResourceViewsMap["Distortion Texture"]));

			// Set the viewport transform.

			mScreenViewport.TopLeftX = 0;
			mScreenViewport.TopLeftY = 0;
			mScreenViewport.Width    = static_cast<float>(mClientWidth);
			mScreenViewport.Height   = static_cast<float>(mClientHeight);
			mScreenViewport.MinDepth = 0.0f;
			mScreenViewport.MaxDepth = 1.0f;

			md3dImmediateContext->RSSetViewports(1, &mScreenViewport);

			mHalfScreenViewport = mScreenViewport;
			mHalfScreenViewport.Width /= 2;
			mHalfScreenViewport.Height /= 2;

			if (riftMan)
				riftMan->getStereo().SetFullViewport(Viewport(0, 0, (int)mScreenViewport.Width, (int)mScreenViewport.Height));

			if (mfxScreenSize)
			{
				float clientSize[2] = {(float)mClientWidth, (float)mClientHeight};
				mfxScreenSize->SetRawValue(&clientSize, 0, 2 * sizeof(float));
			}
		}

		void SetBlurColor(XMFLOAT4 aFloat)
		{
			mfxBlurColor->SetRawValue(&aFloat, 0, sizeof(aFloat));
		}

		ID3D11Device* GetDevice() { return md3dDevice; }

	private:
		int vsync;

		//2D Text variables
		IFW1Factory* pFW1Factory;
		IFW1FontWrapper *pFontWrapper;

		ID3D11Device* md3dDevice;
		ID3D11DeviceContext* md3dImmediateContext;
		IDXGISwapChain* mSwapChain;

		ID3DX11Effect* mFX;
		ID3DX11EffectMatrixVariable* mfxWorld;
		ID3DX11EffectMatrixVariable* mfxViewProj;
		ID3DX11EffectMatrixVariable* mfxOcView;
		ID3DX11EffectMatrixVariable* mfxWorldInvTranspose;
		//Texture stuff
		ID3DX11EffectMatrixVariable* TexTransform;
		ID3DX11EffectVectorVariable* mfxEyePosW;
		ID3DX11EffectVectorVariable* mfxBlurColor;
		ID3DX11EffectVectorVariable* mfxScreenSize;

		ID3DX11EffectVariable* mfxDirLights;
		ID3DX11EffectVariable* mfxPointLights;
		ID3DX11EffectVariable* mfxSpotLight;
		ID3DX11EffectVariable* mfxMaterial;
		ID3DX11EffectVariable* mfxNumLights;

		// Rift stuff
		ID3DX11EffectVectorVariable* mfxLensCenter;
		ID3DX11EffectVectorVariable* mfxScreenCenter;
		ID3DX11EffectVectorVariable* mfxScale;
		ID3DX11EffectVectorVariable* mfxScaleIn;
		ID3DX11EffectVectorVariable* mfxHmdWarpParam;
		ID3DX11EffectVectorVariable* mfxChromAbParam;

		ID3DX11EffectScalarVariable* texelWidth;
		ID3DX11EffectScalarVariable* texelHeight;
		Sky* sky;
		RiftManager* riftMan;
		ID3D11DepthStencilState*	CurDepthState;

		// Maps to various rendering components.
		map<string, XMFLOAT2> diffuseAtlasCoordsMap;
		map<string, ID3D11ShaderResourceView*> shaderResourceViewsMap;
		map<string, ID3D11Texture2D*> texture2DMap;
		map<string, ID3D11RenderTargetView*> renderTargetViewsMap;
		map<string, ID3D11DepthStencilView*> depthStencilViewsMap;
		map<string, ID3DX11EffectTechnique*> techniqueMap;
		map<string, ID3D11RasterizerState*> rasterizerStatesMap;
		map<std::string, unsigned int> instanceCounts;

		ID3DX11EffectShaderResourceVariable* mfxDiffuseMapVar;
		ID3DX11EffectShaderResourceVariable* mfxTextureAtlasVar;
		ID3DX11EffectShaderResourceVariable* mfxSpecMapVar;

		ID3D11InputLayout* mInputLayout;

		XMFLOAT4X4 mWorld;
		//XMFLOAT4X4 mView;
		//XMFLOAT4X4 mProj;
		XMFLOAT4X4 mTexTransform;
		XMFLOAT3 mEyePosW;

		D3D11_VIEWPORT mScreenViewport;
		D3D11_VIEWPORT mHalfScreenViewport;

		D3D_DRIVER_TYPE md3dDriverType;

		unsigned char postProcessingFlags;
		int mClientWidth;
		int mClientHeight;
		int blurCount;
		int totalTextureAtlas;

		bool mEnable4xMsaa;
		bool usingDX11; // If false, we're using DX10 for now.
		UINT m4xMsaaQuality;

		// Holds all the (vertex, index, instanceData) buffers. Separate map due to meshes being constant.
		map<string, BufferPair> bufferPairs;

		// FileLoader.cpp specific things
		//vector<ObjModel> mObjModels;
		TextureManager textureMan;
		vector<GameMaterial> gameMats;
		vector<SurfaceMaterial> surfaceMats;

		// Lights.
		vector<DirectionalLight> mDirLights;
		vector<PointLight> mPointLights;
		SpotLight mSpotLight;

		// Keep a system memory copy of the world matrices for culling.
		map<string, std::vector<InstancedData>> mInstancedDataMap;

		RenderManager() 
		{ 
			vsync = 1;
			md3dDevice = nullptr;
			md3dImmediateContext = nullptr;
			mSwapChain = nullptr;
			mSwapChain = nullptr;
			mFX = nullptr;
			mfxWorld = nullptr;
			mfxViewProj = nullptr;
			mfxOcView = nullptr;
			mfxWorldInvTranspose = nullptr;
			mfxEyePosW = nullptr;
			mfxPointLights = nullptr;
			mfxSpotLight = nullptr;
			mfxMaterial = nullptr;
			mfxNumLights = nullptr;
			mfxBlurColor = nullptr;
			mfxScreenSize = nullptr;
			texelWidth = nullptr;
			texelHeight = nullptr;
			mInputLayout = nullptr;
			mfxLensCenter = nullptr;
			mfxScreenCenter = nullptr;
			mfxScale = nullptr;
			mfxScaleIn = nullptr;
			mfxHmdWarpParam = nullptr;
			mfxChromAbParam = nullptr;
			CurDepthState = nullptr;
			mfxTextureAtlasVar = nullptr;
			sky = nullptr;

			md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

			ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));
			ZeroMemory(&mHalfScreenViewport, sizeof(D3D11_VIEWPORT));
			
			m4xMsaaQuality = 0;
			mEnable4xMsaa = 0;
			totalTextureAtlas = 0;
			blurCount = 1; // Set default to 1 blur when blurring.

			// Set world-view-projection matrix pieces to the identity matrix.
			XMMATRIX I = XMMatrixIdentity();
			XMStoreFloat4x4(&mWorld, I);
			XMStoreFloat4x4(&mTexTransform, I);
			mEyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);

			// Set up lighting. Will need to make more general but first we want basic lighting.
			// Directional light.
			mDirLights.push_back(DirectionalLight());
			mDirLights[0].Ambient  = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
			mDirLights[0].Diffuse  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
			mDirLights[0].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
			mDirLights[0].Direction = XMFLOAT3(0.707f, -0.5f, 0.0f);
			mDirLights.push_back(DirectionalLight());
 			mDirLights[1].Ambient  = XMFLOAT4(0.5f, 0.6f, 0.6f, 1.0f);
 			mDirLights[1].Diffuse  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			mDirLights[1].Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 16.0f);
 			mDirLights[1].Direction = XMFLOAT3(-0.5f, 0.60f, 0.0f);
		}

		~RenderManager()
		{
			//Release 2D Text code
			ReleaseCOM(pFontWrapper);
			ReleaseCOM(pFW1Factory);
			
			ReleaseCOM(mSwapChain);
			ReleaseCOM(mFX);
			ReleaseCOM(mInputLayout);

			// Restore all default settings.
			if( md3dImmediateContext )
				md3dImmediateContext->ClearState();

			ReleaseCOM(md3dImmediateContext);
			ReleaseCOM(md3dDevice);
			
			// Release all the buffers by going through the map.
			map<string, BufferPair>::iterator bufferItr = bufferPairs.begin();
			while (bufferItr != bufferPairs.end())
			{
				ReleaseCOM(bufferItr->second.vertexBuffer);
				ReleaseCOM(bufferItr->second.indexBuffer);
				ReleaseCOM(bufferItr->second.instanceBuffer);
				bufferItr++;
			}

			ReleaseResizeMaps();

			map<string, ID3D11ShaderResourceView*>::iterator srvItr = shaderResourceViewsMap.begin();
			while (srvItr != shaderResourceViewsMap.end())
			{
				ReleaseCOM(srvItr->second);
				srvItr++;
			}

			delete sky;
		}

		// Build any rasterizer states we'll need - right now this is just 'Default' and 'Wireframe'.
		void BuildRasterizerStates()
		{
			D3D11_RASTERIZER_DESC rasterDesc; 
			rasterDesc.AntialiasedLineEnable = false;
			rasterDesc.CullMode = D3D11_CULL_BACK;
			rasterDesc.DepthBias = 0;
			rasterDesc.DepthBiasClamp = 0.0f;
			rasterDesc.DepthClipEnable = true;
			rasterDesc.FillMode = D3D11_FILL_SOLID;
			rasterDesc.FrontCounterClockwise = false;
			rasterDesc.MultisampleEnable = false;
			rasterDesc.ScissorEnable = false;
			rasterDesc.SlopeScaledDepthBias = 0.0f; 
			md3dDevice->CreateRasterizerState ( &rasterDesc , &rasterizerStatesMap["Default"]);
			
			// For Wireframe, we just need to change fill mode.
			rasterDesc.FillMode = D3D11_FILL_WIREFRAME;
			md3dDevice->CreateRasterizerState ( &rasterDesc , &rasterizerStatesMap["Wireframe"]);
		}

		void ToggleWireframe(bool isEnabled)
		{
			md3dImmediateContext->RSSetState ( (isEnabled ? rasterizerStatesMap["Wireframe"] : rasterizerStatesMap["Default"]) ); 
		}

		void UnbindShaderResource(ID3DX11EffectShaderResourceVariable* aResource, string aTechKey)
		{
			aResource->SetResource(nullptr);
			techniqueMap[aTechKey]->GetPassByIndex(0)->Apply(0, md3dImmediateContext);
		}

		// Release all DX components stored in maps that are created via OnResize.
		void ReleaseResizeMaps()
		{
			map<string, ID3D11DepthStencilView*>::iterator depthViewItr = depthStencilViewsMap.begin();
			while (depthViewItr != depthStencilViewsMap.end())
			{
				ReleaseCOM(depthViewItr->second);
				depthViewItr++;
			}

			map<string, ID3D11RenderTargetView*>::iterator renderViewItr = renderTargetViewsMap.begin();
			while (renderViewItr != renderTargetViewsMap.end())
			{
				ReleaseCOM(renderViewItr->second);
				renderViewItr++;
			}

			map<string, ID3D11Texture2D*>::iterator textureItr = texture2DMap.begin();
			while (textureItr != texture2DMap.end())
			{
				ReleaseCOM(textureItr->second);
				textureItr++;
			}

			map<string, ID3D11RasterizerState*>::iterator rastItr = rasterizerStatesMap.begin();
			while (rastItr != rasterizerStatesMap.end())
			{
				ReleaseCOM(rastItr->second);
				++rastItr;
			}

			ReleaseCOM(shaderResourceViewsMap["Default Render Texture"]);
			ReleaseCOM(shaderResourceViewsMap["Blur Output Texture"]);
			ReleaseCOM(shaderResourceViewsMap["Blur Input Texture"]);
		}

		void RenderToEye(StereoEyeParams anEye, Camera* aCamera)
		{
			if (anEye.Eye == StereoEye_Right)
			{
				anEye = riftMan->getLeftEyeParams();
				anEye.Eye = StereoEye_Right;
			}
			Viewport vp = anEye.VP;

			D3D11_VIEWPORT eyeViewport;
			eyeViewport.TopLeftX = (float)vp.x;
			eyeViewport.TopLeftY = (float)vp.y;
			eyeViewport.Width    = (float)vp.w;
			eyeViewport.Height   = (float)vp.h;
			eyeViewport.MinDepth = 0.0f;
			eyeViewport.MaxDepth = 1.0f;

			float	w = float(vp.w) / float(mClientWidth),
					h = float(vp.h) / float(mClientHeight),
					x = float(vp.x) / float(mClientWidth),
					y = float(vp.y) / float(mClientHeight);

			//float scaleFactor = 0.58322453f; //Distortion.Scale;
			float scaleFactor = 1.0f / riftMan->getStereo().GetDistortionConfig().Scale;
			float as = float(vp.w) / float(vp.h);
			float xOffset = riftMan->getStereo().GetDistortionConfig().XCenterOffset;
			if (anEye.Eye == StereoEye_Right)
				xOffset *= -1;
			float aLens[2] = {x + (w + xOffset /*0.15197642f*/ * 0.5f)*0.5f, y + h*0.5f};
			float aScale[2] = { (w/2) * scaleFactor, (h/2) * scaleFactor * as };
			float aScaleIn[2] = { (2/w),(2/h) / as };
			float screenCenter[2] = {x + w*0.5f, y + h*0.5f};
				
			mfxLensCenter->SetRawValue(aLens, 0, 2 * sizeof(float));
			mfxScale->SetRawValue(aScale, 0, 2 * sizeof(float));
			mfxScaleIn->SetRawValue(aScaleIn, 0, 2 * sizeof(float));
			mfxScreenCenter->SetRawValue(screenCenter, 0, 2 * sizeof(float));
			mfxHmdWarpParam->SetRawValue(riftMan->getStereo().GetDistortionConfig().K, 0, 4 * sizeof(float));
			mfxChromAbParam->SetRawValue(riftMan->getStereo().GetDistortionConfig().ChromaticAberration, 0, 4 * sizeof(float));
				
			md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Distortion Texture"], depthStencilViewsMap["Distortion"]);
			md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Distortion"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
				
			XMMATRIX view = aCamera->View();
			// Recalculate projection to use left-hand coordinate system.

			// FOV might need to be tweaked, and needs testing.
			Matrix4f projCenter = Matrix4f::PerspectiveLH(riftMan->getStereo().GetYFOVRadians(), aCamera->GetAspect() * riftMan->getStereo().GetAspectMultiplier(), 
								aCamera->GetNearZ(), aCamera->GetFarZ());
			//Matrix4f projCenter = Matrix4f::PerspectiveLH(aCamera->GetFovY(), aCamera->GetAspect() * riftMan->getStereo().GetAspectMultiplier(), 
									//aCamera->GetNearZ(), aCamera->GetFarZ());

			float projOffset = riftMan->getStereo().GetProjectionCenterOffset();
			if (anEye.Eye == StereoEye_Right)
				projOffset *= -1;

			XMMATRIX projShift = XMMATRIX(reinterpret_cast<float*>( (Matrix4f::Translation(projOffset, 0, 0) * projCenter).M) );
			float intDist = riftMan->getStereo().GetHMDInfo().InterpupillaryDistance;
			if (anEye.Eye == StereoEye_Left)
				intDist *= -1;
			XMMATRIX viewAdjust = XMMATRIX(reinterpret_cast<float*>(Matrix4f::Translation(Vector3f(intDist * 0.5f,0,0)).M));

			view = XMMatrixMultiply(view, viewAdjust);

			D3D11_VIEWPORT tLV = eyeViewport;
			tLV.Width = (float)(int)ceil(riftMan->getStereo().GetDistortionScale() * tLV.Width);
			tLV.Height = (float)(int)ceil(riftMan->getStereo().GetDistortionScale() * tLV.Height);
			tLV.TopLeftX = (float)(int)ceil(riftMan->getStereo().GetDistortionScale() * tLV.TopLeftX);
			tLV.TopLeftY = (float)(int)ceil(riftMan->getStereo().GetDistortionScale() * tLV.TopLeftY);
			
			//tLV.TopLeftX = 0;	
			md3dImmediateContext->RSSetViewports(1, &tLV);

			const float* viewproj = reinterpret_cast<const float*>(&XMMatrixMultiply( view, XMMatrixTranspose(projShift) ));

			mEyePosW = aCamera->GetPosition();
			mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
			mfxViewProj->SetMatrix(viewproj); 
			DrawGameObjects("LightsWithAtlas");
			UnbindShaderResource(mfxDiffuseMapVar, "LightsWithAtlas");
			if(anEye.Eye == StereoEye_Right)
				sky->Draw(md3dImmediateContext, *aCamera, viewproj);
			else
				sky->Draw(md3dImmediateContext, *aCamera, viewproj);

			md3dImmediateContext->IASetInputLayout(mInputLayout);
			md3dImmediateContext->OMSetDepthStencilState(0, 0);

			if (anEye.Eye == StereoEye_Right)
				eyeViewport.TopLeftX += eyeViewport.Width;

			md3dImmediateContext->RSSetViewports(1, &eyeViewport);

			Matrix4f texm(w, 0.0f, 0.0f, x,
								0.0f, h, 0.0f, y,
								0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);

			Matrix4f ocView(1.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 1.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 0.0f,
								0.0f, 0.0f, 0.0f, 1.0f);

			//mfxOcView->SetMatrix(reinterpret_cast<const float*>(&XMMatrixIdentity()));
			mfxOcView->SetMatrix(reinterpret_cast<const float*>(&ocView));

			TexTransform->SetMatrix(reinterpret_cast<const float*>(&texm));
			//TexTransform->SetMatrix(reinterpret_cast<const float*>(&XMMatrixIdentity()));

			md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Default Render Texture"], depthStencilViewsMap["Default"]);
			md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Default"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);
			mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Distortion Texture"]);

			DrawQuad("OculusTech");
			UnbindShaderResource(mfxDiffuseMapVar, "OculusTech");

			//DrawQuad("TexturePassThrough");
			//UnbindShaderResource(mfxDiffuseMapVar, "TexturePassThrough");
		}

		RenderManager(RenderManager const&); // Don't implement.
		void operator=(RenderManager const&); // Don't implement.
};
#endif
/* Uncomment to re-induce nausea
// Bind the render target view to the back buffer.
md3dImmediateContext->OMSetRenderTargets(1, &renderTargetViewsMap["Back Buffer"], depthStencilViewsMap["Default"]);
		
// Clear the render target and depth/stencil view.
md3dImmediateContext->ClearRenderTargetView(renderTargetViewsMap["Back Buffer"], reinterpret_cast<const float*>(&Colors::LightSteelBlue));
md3dImmediateContext->ClearDepthStencilView(depthStencilViewsMap["Default"], D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

// Set texture atlas once for now.
mfxDiffuseMapVar->SetResource(shaderResourceViewsMap["Default Render Texture"]);

DrawGameObjects("LightsWithoutAtlas");
*/

/*


XMMATRIX projInRH(proj.m[0][0], proj.m[0][2], proj.m[0][1], proj.m[0][3],
					proj.m[2][0], proj.m[2][2], proj.m[2][1], proj.m[1][3],
					proj.m[1][0], proj.m[1][2], proj.m[1][1], proj.m[2][3],
					proj.m[3][0], proj.m[3][2], proj.m[3][1], proj.m[3][3]);

XMMATRIX viewAdjInRH(viewAdjust.m[0][0], viewAdjust.m[0][2], viewAdjust.m[0][1], viewAdjust.m[0][3],
					viewAdjust.m[2][0], viewAdjust.m[2][2], viewAdjust.m[2][1], viewAdjust.m[1][3],
					viewAdjust.m[1][0], viewAdjust.m[1][2], viewAdjust.m[1][1], viewAdjust.m[2][3],
					viewAdjust.m[3][0], viewAdjust.m[3][2], viewAdjust.m[3][1], viewAdjust.m[3][3]);
*/