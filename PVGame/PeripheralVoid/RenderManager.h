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

		float AspectRatio() const
		{
			return static_cast<float>(mClientWidth) / mClientHeight;
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

			return true;
		}

		int GetClientWidth() { return mClientWidth; }
		int GetClientHeight() { return mClientHeight; }
		
		void DrawMenu()
		{
			// Pretty self-explanatory. Clears the screen, essentially.
			md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::Silver));
			HR(mSwapChain->Present(0, 0));
		}

		void DrawScene(Camera* aCamera, vector<GameObject*> gameObjects)
		{
			// Pretty self-explanatory. Clears the screen, essentially.
			md3dImmediateContext->ClearRenderTargetView(mRenderTargetView, reinterpret_cast<const float*>(&Colors::LightSteelBlue));
			md3dImmediateContext->ClearDepthStencilView(mDepthStencilView, D3D11_CLEAR_DEPTH|D3D11_CLEAR_STENCIL, 1.0f, 0);

			/*
			ID3D11RasterizerState* pRSwireFrame; 
			D3D11_RASTERIZER_DESC RSWireFrameDesc; 
			RSWireFrameDesc.FillMode = D3D11_FILL_WIREFRAME; 
			RSWireFrameDesc.CullMode = D3D11_CULL_NONE;
 
			md3dDevice->CreateRasterizerState ( &RSWireFrameDesc , &pRSwireFrame );  
			// Assumes that "pDevice" is valid (ID3D11Device*) 
 
			md3dImmediateContext->RSSetState ( pRSwireFrame ); 
			*/

			// Sets input layout which describes the vertices we're about to send.
			md3dImmediateContext->IASetInputLayout(mInputLayout);
			// Triangle list = every 3 vertices is a SEPERATE triangle.
			md3dImmediateContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			UINT stride = sizeof(Vertex);
			UINT offset = 0;
			mEyePosW = aCamera->GetPosition();

			// Set per frame constants.
			mfxDirLights->SetRawValue(&mDirLights[0], 0, sizeof(DirectionalLight) * mDirLights.size());
			mfxPointLights->SetRawValue(&mPointLights[0], 0, 4 * sizeof(PointLight));
			mfxSpotLight->SetRawValue(&mSpotLight, 0, sizeof(mSpotLight));
			mfxEyePosW->SetRawValue(&mEyePosW, 0, sizeof(mEyePosW));
			//Set the resources for the shader resource view
			
			TexTransform->SetMatrix(reinterpret_cast<const float*>(&mTexTransform));
			
			D3DX11_TECHNIQUE_DESC techDesc;
			mTech->GetDesc( &techDesc );

			// Optimization strategy. We use the same buffer a lot, so we can save the info we need per buffer and only change it each time we use a new buffer.
			string prevKey = "";
			D3D11_BUFFER_DESC buffDesc;
			D3D11_BUFFER_DESC indexDesc;

			// How many indices / vertices there are.
			int indexSize = 0;
			int vertexSize = 0;

			for (UINT i = 0; i < gameObjects.size(); i++)
			{
				for(UINT p = 0; p < techDesc.Passes; ++p)
				{
					string currentKey = gameObjects[i]->GetMeshKey();
					// If this is a new buffer key, set all of the relevant info based on that buffer.
					if (prevKey.compare(currentKey) != 0)
					{
						md3dImmediateContext->IASetVertexBuffers(0, 1, &bufferPairs[currentKey].vertexBuffer, &stride, &offset);
						md3dImmediateContext->IASetIndexBuffer(bufferPairs[currentKey].indexBuffer, DXGI_FORMAT_R32_UINT, 0);
						bufferPairs[currentKey].vertexBuffer->GetDesc(&buffDesc);
						bufferPairs[currentKey].indexBuffer->GetDesc(&indexDesc);
						indexSize = indexDesc.ByteWidth / sizeof(UINT);
						vertexSize = buffDesc.ByteWidth / sizeof(Vertex);
						prevKey = currentKey;
					}
					
					// Get the material via the GameObject's key. We then set surface material, diffuse map, etc, by using the keys in the map.
					GameMaterial aMaterial = GAME_MATERIALS[gameObjects[i]->GetMaterialKey()];

					/*D3D11_BUFFER_DESC buffDesc;
					bufferPairs[currentKey].vertexBuffer->GetDesc(&buffDesc);
					D3D11_BUFFER_DESC indexDesc;
					bufferPairs[currentKey].indexBuffer->GetDesc(&indexDesc);

					int indexSize = indexDesc.ByteWidth / sizeof(UINT);
					int vertexSize = buffDesc.ByteWidth / sizeof(Vertex);*/

					// Set constants
					XMMATRIX world = XMLoadFloat4x4(&gameObjects[i]->GetWorldMatrix());
					XMMATRIX worldInvTranspose = MathHelper::InverseTranspose(world); // For transforming normals.
					XMMATRIX worldViewProj = world * aCamera->ViewProj();

					// The shader variables per-object.
					mfxWorld->SetMatrix(reinterpret_cast<float*>(&world));
					mfxWorldInvTranspose->SetMatrix(reinterpret_cast<float*>(&worldInvTranspose));
					mfxWorldViewProj->SetMatrix(reinterpret_cast<float*>(&worldViewProj));
					
					mfxDiffuseMapVar->SetResource(diffuseMaps[aMaterial.DiffuseKey]);

					SurfaceMaterial aSurfaceMaterial = SURFACE_MATERIALS[aMaterial.SurfaceKey];
					mfxMaterial->SetRawValue(&aSurfaceMaterial, 0, sizeof(aSurfaceMaterial));

					mTech->GetPassByIndex(p)->Apply(0, md3dImmediateContext);
					md3dImmediateContext->DrawIndexed(indexSize, 0, 0);
				}
			}
			HR(mSwapChain->Present(0, 0));
		}
		
		// Build a buffer for each mesh.
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
			diffuseMaps[aKey] = aShaderResourceView;
		}

		void BuildFX()
		{
			std::ifstream fin("fx/Basic.fxo", std::ios::binary);

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
				mTech                = mFX->GetTechniqueByName("TestLights");
			else
				mTech                = mFX->GetTechniqueByName("TestLightsDX10");

			// Creates association between shader variables and program variables.
			mfxWorldViewProj     = mFX->GetVariableByName("gWorldViewProj")->AsMatrix();
			mfxWorld             = mFX->GetVariableByName("gWorld")->AsMatrix();
			mfxWorldInvTranspose = mFX->GetVariableByName("gWorldInvTranspose")->AsMatrix();
			mfxEyePosW           = mFX->GetVariableByName("gEyePosW")->AsVector();
			mfxDirLights          = mFX->GetVariableByName("gDirLights");
			mfxPointLights        = mFX->GetVariableByName("testLights");
			mfxSpotLight         = mFX->GetVariableByName("gSpotLight");
			mfxMaterial          = mFX->GetVariableByName("gMaterial");
			mfxDiffuseMapVar	 = mFX->GetVariableByName("gDiffuseMap")->AsShaderResource();
			mfxSpecMapVar		 = mFX->GetVariableByName("gSpecMap")->AsShaderResource();
			TexTransform		 = mFX->GetVariableByName("gTexTransform")->AsMatrix();
		}

		void BuildVertexLayout()
		{
			// Create the vertex input layout.
			D3D11_INPUT_ELEMENT_DESC vertexDesc[] =
			{
				{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"NORMAL",    0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0},
				{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0}
			};

			// Create the input layout
			D3DX11_PASS_DESC passDesc;
			mTech->GetPassByIndex(0)->GetDesc(&passDesc);
			HR(md3dDevice->CreateInputLayout(vertexDesc, 3, passDesc.pIAInputSignature, passDesc.IAInputSignatureSize, &mInputLayout));
		}

		void ToggleLight(int index)
		{
			mPointLights[index].On.x = (mPointLights[index].On.x == 0) ? 1 : 0;
		}
		
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

			ReleaseCOM(mRenderTargetView);
			ReleaseCOM(mDepthStencilView);
			ReleaseCOM(mDepthStencilBuffer);


			// Resize the swap chain and recreate the render target view.

			HR(mSwapChain->ResizeBuffers(1, mClientWidth, mClientHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 0));
			ID3D11Texture2D* backBuffer;
			HR(mSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuffer)));
			HR(md3dDevice->CreateRenderTargetView(backBuffer, 0, &mRenderTargetView));
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

			HR(md3dDevice->CreateTexture2D(&depthStencilDesc, 0, &mDepthStencilBuffer));
			HR(md3dDevice->CreateDepthStencilView(mDepthStencilBuffer, 0, &mDepthStencilView));


			// Bind the render target view and depth/stencil view to the pipeline.

			md3dImmediateContext->OMSetRenderTargets(1, &mRenderTargetView, mDepthStencilView);
	

			// Set the viewport transform.

			mScreenViewport.TopLeftX = 0;
			mScreenViewport.TopLeftY = 0;
			mScreenViewport.Width    = static_cast<float>(mClientWidth);
			mScreenViewport.Height   = static_cast<float>(mClientHeight);
			mScreenViewport.MinDepth = 0.0f;
			mScreenViewport.MaxDepth = 1.0f;

			md3dImmediateContext->RSSetViewports(1, &mScreenViewport);
		}

		ID3D11Device* GetDevice() { return md3dDevice; }

	private:
		ID3D11Device* md3dDevice;
		ID3D11DeviceContext* md3dImmediateContext;
		IDXGISwapChain* mSwapChain;
		ID3D11Texture2D* mDepthStencilBuffer;
		ID3D11RenderTargetView* mRenderTargetView;
		ID3D11DepthStencilView* mDepthStencilView;

		ID3DX11Effect* mFX;
		ID3DX11EffectTechnique* mTech;
		ID3DX11EffectMatrixVariable* mfxWorld;
		ID3DX11EffectMatrixVariable* mfxWorldViewProj;
		ID3DX11EffectMatrixVariable* mfxWorldInvTranspose;
		//Texture stuff
		ID3DX11EffectMatrixVariable* TexTransform;
		ID3DX11EffectVectorVariable* mfxEyePosW;
		ID3DX11EffectVariable* mfxDirLights;
		ID3DX11EffectVariable* mfxPointLights;
		ID3DX11EffectVariable* mfxSpotLight;
		ID3DX11EffectVariable* mfxMaterial;

		map<string, ID3D11ShaderResourceView*> diffuseMaps;
		
		ID3DX11EffectShaderResourceVariable* mfxDiffuseMapVar;
		ID3DX11EffectShaderResourceVariable* mfxSpecMapVar;
		ID3DX11EffectShaderResourceVariable* DiffuseMap;

		ID3D11InputLayout* mInputLayout;

		XMFLOAT4X4 mWorld;
		//XMFLOAT4X4 mView;
		//XMFLOAT4X4 mProj;
		XMFLOAT4X4 mTexTransform;
		XMFLOAT3 mEyePosW;

		D3D11_VIEWPORT mScreenViewport;
		D3D_DRIVER_TYPE md3dDriverType;

		int mClientWidth;
		int mClientHeight;

		bool mEnable4xMsaa;
		bool usingDX11; // If false, we're using DX10 for now.
		UINT m4xMsaaQuality;

		// Holds all the (vertex, index) buffers. Separate map due to meshes being constant.
		map<string, BufferPair> bufferPairs;

		// Lights.
		vector<DirectionalLight> mDirLights;
		vector<PointLight> mPointLights;
		SpotLight mSpotLight;

		RenderManager() 
		{ 
			md3dDevice = nullptr;
			md3dImmediateContext = nullptr;
			mSwapChain = nullptr;
			mSwapChain = nullptr;
			mDepthStencilBuffer = nullptr;
			mRenderTargetView = nullptr;
			mFX = nullptr;
			mTech = nullptr;
			mfxWorld = nullptr;
			mfxWorldViewProj = nullptr;
			mfxWorldInvTranspose = nullptr;
			mfxEyePosW = nullptr;
			mfxPointLights = nullptr;
			mfxSpotLight = nullptr;
			mfxMaterial = nullptr;

			mInputLayout = nullptr;
			md3dDriverType = D3D_DRIVER_TYPE_HARDWARE;

			ZeroMemory(&mScreenViewport, sizeof(D3D11_VIEWPORT));
			m4xMsaaQuality = 0;
			mEnable4xMsaa = 0;

			// Set world-view-projection matrix pieces to the identity matrix.
			XMMATRIX I = XMMatrixIdentity();
			XMStoreFloat4x4(&mWorld, I);
			XMStoreFloat4x4(&mTexTransform, I);
			mEyePosW = XMFLOAT3(0.0f, 0.0f, 0.0f);

			// Set up lighting. Will need to make more general but first we want basic lighting.
			// Directional light.
			mDirLights.push_back(DirectionalLight());
			mDirLights[0].Ambient  = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
			mDirLights[0].Diffuse  = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
			mDirLights[0].Specular = XMFLOAT4(0.6f, 0.6f, 0.6f, 16.0f);
			mDirLights[0].Direction = XMFLOAT3(0.707f, -0.707f, 0.0f);
 
			//mDirLights[1].Ambient  = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			//mDirLights[1].Diffuse  = XMFLOAT4(1.4f, 1.4f, 1.4f, 1.0f);
			//mDirLights[1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 16.0f);
			//mDirLights[1].Direction = XMFLOAT3(-0.707f, 0.0f, 0.707f);

			// Add 4 lights to the scene. First is red.
			PointLight aPointLight;
			aPointLight.Ambient = XMFLOAT4(4.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Range = 2.0f;
			aPointLight.Position = XMFLOAT3(-1.5f, 0.0f, -1.5f);
			aPointLight.Att = XMFLOAT3(0.0f, 0.0f, 1.0f);
			aPointLight.On = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
			mPointLights.push_back(PointLight(aPointLight));

			// Second is green.
			aPointLight.Ambient = XMFLOAT4(0.0f, 4.0f, 0.0f, 1.0f);
			aPointLight.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Range = 2.0f;
			aPointLight.Position = XMFLOAT3(1.5f, 0.0f, 1.5f);
			aPointLight.Att = XMFLOAT3(0.0f, 0.0f, 1.0f);
			aPointLight.On = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
			mPointLights.push_back(PointLight(aPointLight));

			// Third is blue.
			PointLight bPointLight;
			bPointLight.Ambient = XMFLOAT4(0.0f, 0.0f, 4.0f, 1.0f);
			bPointLight.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			bPointLight.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			bPointLight.Range = 2.0f;
			bPointLight.Position = XMFLOAT3(1.5f, 0.0f, -1.5f);
			bPointLight.Att = XMFLOAT3(0.0f, 0.0f, 1.0f);
			bPointLight.On = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
			mPointLights.push_back(PointLight(bPointLight));

			// Fourth is purple. 
			aPointLight.Ambient = XMFLOAT4(4.0f, 0.0f, 4.0f, 1.0f);
			aPointLight.Diffuse = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Specular = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
			aPointLight.Range = 2.0f;
			aPointLight.Position = XMFLOAT3(-1.5f, 0.0f, 1.5f);
			aPointLight.Att = XMFLOAT3(0.0f, 0.0f, 1.0f);
			aPointLight.On = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
			mPointLights.push_back(PointLight(aPointLight));
		}

		~RenderManager()
		{
			ReleaseCOM(mRenderTargetView);
			ReleaseCOM(mDepthStencilView);
			ReleaseCOM(mSwapChain);
			ReleaseCOM(mDepthStencilBuffer);
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
				bufferItr++;
			}

			map<string, ID3D11ShaderResourceView*>::iterator diffuseItr = diffuseMaps.begin();
			while (diffuseItr != diffuseMaps.end())
			{
				ReleaseCOM(diffuseItr->second);
				diffuseItr++;
			}
		}

		RenderManager(RenderManager const&); // Don't implement.
		void operator=(RenderManager const&); // Don't implement.
};

#endif