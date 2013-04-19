//***************************************************************************************
// Camera.h by Frank Luna (C) 2011 All Rights Reserved.
//***************************************************************************************

#include "Camera.h"

Camera::Camera(PhysicsManager* pM)
	: mPosition(0.0f, 0.0f, 0.0f), 
	  mRight(1.0f, 0.0f, 0.0f),
	  mUp(0.0f, 1.0f, 0.0f),
	  mLook(0.0f, 0.0f, 1.0f)
{
	physicsMan = pM;
	SetLens(0.25f*MathHelper::Pi, 1.0f, 0.01f, 1000.0f);
}

Camera::~Camera()
{
#if USE_FRUSTUM_CULLING
		physicsMan->removeGhostObjectFromWorld(body);
#endif
}

btPairCachingGhostObject* Camera::getBody()
{
	return body;
}

XMVECTOR Camera::GetPositionXM()const
{
	return XMLoadFloat3(&mPosition);
}

XMFLOAT3 Camera::GetPosition()const
{
	return mPosition;
}

void Camera::SetPosition(float x, float y, float z)
{
	mPosition = XMFLOAT3(x, y, z);
	if(USE_FRUSTUM_CULLING)
		transformBody();
}

void Camera::SetPosition(const XMFLOAT3& v)
{
	mPosition = v;
}

XMVECTOR Camera::GetRightXM()const
{
	return XMLoadFloat3(&mRight);
}

XMFLOAT3 Camera::GetRight()const
{
	return mRight;
}

XMVECTOR Camera::GetUpXM()const
{
	return XMLoadFloat3(&mUp);
}

XMFLOAT3 Camera::GetUp()const
{
	return mUp;
}

XMVECTOR Camera::GetLookXM()const
{
	return XMLoadFloat3(&mLook);
}

XMFLOAT3 Camera::GetLook()const
{
	return mLook;
}

float Camera::GetNearZ()const
{
	return mNearZ;
}

float Camera::GetFarZ()const
{
	return mFarZ;
}

float Camera::GetAspect()const
{
	return mAspect;
}

float Camera::GetFovY()const
{
	return mFovY;
}

float Camera::GetFovX()const
{
	float halfWidth = 0.5f*GetNearWindowWidth();
	return 2.0f*atan(halfWidth / mNearZ);
}

float Camera::GetNearWindowWidth()const
{
	return mAspect * mNearWindowHeight;
}

float Camera::GetNearWindowHeight()const
{
	return mNearWindowHeight;
}

float Camera::GetFarWindowWidth()const
{
	return mAspect * mFarWindowHeight;
}

float Camera::GetFarWindowHeight()const
{
	return mFarWindowHeight;
}

void Camera::SetLens(float fovY, float aspect, float zn, float zf)
{
	// cache properties
	mFovY = fovY;
	mAspect = aspect;
	mNearZ = zn;
	mFarZ = zf;

	mNearWindowHeight = 2.0f * mNearZ * tanf( 0.5f*mFovY );
	mFarWindowHeight  = 2.0f * mFarZ * tanf( 0.5f*mFovY );
	
	XMMATRIX P = XMMatrixPerspectiveFovLH(mFovY, mAspect, mNearZ, mFarZ);
	XMStoreFloat4x4(&mProj, P);

#if USE_FRUSTUM_CULLING
		float mNearWindowWidth = 2.0f * mNearZ * tanf( 0.5f*mAspect );
		float mFarWindowWidth  = 2.0f * mFarZ * tanf( 0.5f*mAspect );

		btVector3* points = new btVector3[8];
		//NearPlane
		points[0] = btVector3( mNearWindowWidth/2,  mNearWindowHeight/2, mNearZ);
		points[1] = btVector3(-mNearWindowWidth/2,  mNearWindowHeight/2, mNearZ);
		points[2] = btVector3( mNearWindowWidth/2, -mNearWindowHeight/2, mNearZ);
		points[3] = btVector3(-mNearWindowWidth/2, -mNearWindowHeight/2, mNearZ);

		points[4] = btVector3( mFarWindowWidth/2,  mFarWindowHeight/2, mFarZ);
		points[5] = btVector3(-mFarWindowWidth/2,  mFarWindowHeight/2, mFarZ);
		points[6] = btVector3( mFarWindowWidth/2, -mFarWindowHeight/2, mFarZ);
		points[7] = btVector3(-mFarWindowWidth/2, -mFarWindowHeight/2, mFarZ);

		body = physicsMan->makeCameraFrustumObject(points, 8);
		
		physicsMan->addGhostObjectToWorld(body);

	#if DRAW_FRUSTUM
		//Near Plane
		btVector3 nTR(  mNearWindowWidth/2,  mNearWindowHeight/2, mNearZ);
		btVector3 nTL( -mNearWindowWidth/2,  mNearWindowHeight/2, mNearZ);
		btVector3 nBR(  mNearWindowWidth/2, -mNearWindowHeight/2, mNearZ);
		btVector3 nBL( -mNearWindowWidth/2, -mNearWindowHeight/2, mNearZ);

		//Far Plane
		btVector3 fTR(  mFarWindowWidth/2,  mFarWindowHeight/2, mFarZ);
		btVector3 fTL( -mFarWindowWidth/2,  mFarWindowHeight/2, mFarZ);
		btVector3 fBR(  mFarWindowWidth/2, -mFarWindowHeight/2, mFarZ);
		btVector3 fBL( -mFarWindowWidth/2, -mFarWindowHeight/2, mFarZ);

		Vertex frustumVertices[] =
		{
			// Normals derived by hand - will want to be more efficient later.
			{ XMFLOAT3(nTR.getX(), nTR.getY(), nTR.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f,  1.0f)	},	//nTR
			{ XMFLOAT3(nTL.getX(), nTL.getY(), nTL.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2( 1.0f,  0.0f)	},	//nTL
			{ XMFLOAT3(nBL.getX(), nBL.getY(), nBL.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f, -1.0f)	},	//nBL
			{ XMFLOAT3(nBR.getX(), nBR.getY(), nBR.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(-1.0f,  0.0f)	},	//nBR

			{ XMFLOAT3(fTR.getX(), fTR.getY(), fTR.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f,  1.0f)	},	//fTR
			{ XMFLOAT3(fTL.getX(), fTL.getY(), fTL.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2( 1.0f,  0.0f)	},	//fTL
			{ XMFLOAT3(fBL.getX(), fBL.getY(), fBL.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2( 0.0f, -1.0f)	},	//fBL
			{ XMFLOAT3(fBR.getX(), fBR.getY(), fBR.getZ()), XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT2(-1.0f,  0.0f)	},	//fBR
		
		};
		MeshMaps::MESH_MAPS["Frustum"].vertices.assign(frustumVertices, frustumVertices + 8);

		btTriangleMesh* tMesh = new btTriangleMesh();
	
		//Near
		tMesh->addTriangle(nTR, nTL, nBL);
		tMesh->addTriangle(nTR, nBR, nBL);

		//Top
		tMesh->addTriangle(nTL, fTL, fTR);
		tMesh->addTriangle(nTL, nTR, fTR);

		//Left
		tMesh->addTriangle(nTL, fTL, fBL);
		tMesh->addTriangle(nTL, nBL, fBL);

		//Bottom
		tMesh->addTriangle(nBR, nBL, fBL);
		tMesh->addTriangle(nBR, fBR, fBL);

		//Right
		tMesh->addTriangle(nBR, nTR, fTR);
		tMesh->addTriangle(nBR, fBR, fTR);
	
		//Far
		tMesh->addTriangle(fTR, fTL, fBL);
		tMesh->addTriangle(fTR, fBR, fBL);


		// Create the index buffer
		UINT planeIndices[] = 
		{
			//Near
			0, 1, 2,
			0, 3, 2,

			//Top
			1, 5, 4,
			1, 0, 4,

			//Left
			1, 5, 6,
			1, 2, 6,

			//Bottom
			3, 2, 6,
			3, 7, 6,

			//Right
			3, 0, 4,
			3, 7, 4,
	
			//Far
			4, 5, 6,
			4, 7, 6
		};
		MeshMaps::MESH_MAPS["Frustum"].indices.assign(planeIndices, planeIndices + 36);
		MeshMaps::MESH_MAPS["Frustum"].bufferKey = "Frustum";
		MeshMaps::MESH_MAPS["Frustum"].normalizeVertices = false;

		physicsMan->addTriangleMesh("Frustum", MeshMaps::MESH_MAPS["Frustum"]);
		
		frustumBody = new GameObject("Frustum", "Test Wall", physicsMan->createRigidBody("Frustum", 0,4,0), physicsMan);
		frustumBody->addCollisionFlags(btCollisionObject::CollisionFlags::CF_NO_CONTACT_RESPONSE);
		frustumBody->CalculateWorldMatrix();
	#endif //DRAW_FRUSTUM
#endif //USE_FRUSTUM_CULLING
}

void Camera::LookAt(FXMVECTOR pos, FXMVECTOR target, FXMVECTOR worldUp)
{
	XMVECTOR L = XMVector3Normalize(XMVectorSubtract(target, pos));
	XMVECTOR R = XMVector3Normalize(XMVector3Cross(worldUp, L));
	XMVECTOR U = XMVector3Cross(L, R);

	XMStoreFloat3(&mPosition, pos);
	XMStoreFloat3(&mLook, L);
	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
#if USE_FRUSTUM_CULLING
		transformBody();
#endif
}

void Camera::LookAt(const XMFLOAT3& pos, const XMFLOAT3& target, const XMFLOAT3& up)
{
	XMVECTOR P = XMLoadFloat3(&pos);
	XMVECTOR T = XMLoadFloat3(&target);
	XMVECTOR U = XMLoadFloat3(&up);

	LookAt(P, T, U);
}

XMMATRIX Camera::View()const
{
	return XMLoadFloat4x4(&mView);
}

XMMATRIX Camera::Proj()const
{
	return XMLoadFloat4x4(&mProj);
}

XMMATRIX Camera::ViewProj()const
{
	return XMMatrixMultiply(View(), Proj());
}

void Camera::Strafe(float d)
{
	// mPosition += d*mRight
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR r = XMLoadFloat3(&mRight);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, r, p));
#if USE_FRUSTUM_CULLING
		transformBody();
#endif
}

void Camera::Walk(float d)
{
	// mPosition += d*mLook
	XMVECTOR s = XMVectorReplicate(d);
	XMVECTOR l = XMLoadFloat3(&mLook);
	XMVECTOR p = XMLoadFloat3(&mPosition);
	XMStoreFloat3(&mPosition, XMVectorMultiplyAdd(s, l, p));
	if(USE_FRUSTUM_CULLING)
		transformBody();
}

void Camera::Pitch(float angle)
{
	// Rotate up and look vector about the right vector.

	XMMATRIX R = XMMatrixRotationAxis(XMLoadFloat3(&mRight), angle);

	XMStoreFloat3(&mUp,   XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
#if USE_FRUSTUM_CULLING
		transformBody();
#endif
}

void Camera::RotateY(float angle)
{
	// Rotate the basis vectors about the world y-axis.

	XMMATRIX R = XMMatrixRotationY(angle);

	XMStoreFloat3(&mRight,   XMVector3TransformNormal(XMLoadFloat3(&mRight), R));
	XMStoreFloat3(&mUp, XMVector3TransformNormal(XMLoadFloat3(&mUp), R));
	XMStoreFloat3(&mLook, XMVector3TransformNormal(XMLoadFloat3(&mLook), R));
	if(USE_FRUSTUM_CULLING)
		transformBody();
}

void Camera::UpdateViewMatrix()
{
	XMVECTOR R = XMLoadFloat3(&mRight);
	XMVECTOR U = XMLoadFloat3(&mUp);
	XMVECTOR L = XMLoadFloat3(&mLook);
	XMVECTOR P = XMLoadFloat3(&mPosition);

	// Keep camera's axes orthogonal to each other and of unit length.
	L = XMVector3Normalize(L);
	U = XMVector3Normalize(XMVector3Cross(L, R));

	// U, L already ortho-normal, so no need to normalize cross product.
	R = XMVector3Cross(U, L); 

	// Fill in the view matrix entries.
	float x = -XMVectorGetX(XMVector3Dot(P, R));
	float y = -XMVectorGetX(XMVector3Dot(P, U));
	float z = -XMVectorGetX(XMVector3Dot(P, L));

	XMStoreFloat3(&mRight, R);
	XMStoreFloat3(&mUp, U);
	XMStoreFloat3(&mLook, L);

	mView(0,0) = mRight.x; 
	mView(1,0) = mRight.y; 
	mView(2,0) = mRight.z; 
	mView(3,0) = x;   

	mView(0,1) = mUp.x;
	mView(1,1) = mUp.y;
	mView(2,1) = mUp.z;
	mView(3,1) = y;  

	mView(0,2) = mLook.x; 
	mView(1,2) = mLook.y; 
	mView(2,2) = mLook.z; 
	mView(3,2) = z;   

	mView(0,3) = 0.0f;
	mView(1,3) = 0.0f;
	mView(2,3) = 0.0f;
	mView(3,3) = 1.0f;
	
#if USE_FRUSTUM_CULLING
		transformBody();
#endif
}

void Camera::transformBody()
{
	btTransform t = body->getWorldTransform();
	t.setOrigin(btVector3(mPosition.x, mPosition.y, mPosition.z));

	XMVECTOR quat = XMQuaternionRotationMatrix(View());
	XMFLOAT4 fQuat;
	XMStoreFloat4(&fQuat, quat);
	btQuaternion ret(fQuat.x, fQuat.y, fQuat.z, -fQuat.w);
	t.setRotation(ret);

	body->setWorldTransform(t);

#if DRAW_FRUSTUM
		btTransform t2 = frustumBody->getRigidBody()->getWorldTransform();
	
		XMFLOAT3 cPos(mPosition.x + (mLook.x *2),mPosition.y + (mLook.y * 2), mPosition.z + (mLook.z * 2));
		t2.setOrigin(btVector3(cPos.x, cPos.y, cPos.z));

		XMVECTOR quat2 = XMQuaternionRotationMatrix(View());
		XMFLOAT4 fQuat2;
		XMStoreFloat4(&fQuat2, quat2);
		btQuaternion ret2(fQuat2.x, fQuat2.y, fQuat2.z, -fQuat2.w);
		t2.setRotation(ret2);
	
		frustumBody->getRigidBody()->setWorldTransform(t2);
		frustumBody->getRigidBody()->getMotionState()->setWorldTransform(t2);
		frustumBody->CalculateWorldMatrix();
#endif
}

void Camera::frustumCull()
{
	physicsMan->frustumCulling(body);
}