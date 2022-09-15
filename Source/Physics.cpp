#include "Physics.h"
#include <cmath>
#include "LoggerMacros.h"
#include <algorithm>



namespace FLOOF {
	bool Physics::TriangleBallIntersect(const FLOOF::Triangle& triangle, const glm::vec3& position, const float& radius) {
		auto intersect = (glm::dot(position - triangle.A, triangle.N));
		return (intersect < radius);
	}

	glm::vec3 Physics::GetVelocityBall(const FLOOF::Triangle& triangle, const glm::vec3& position,
			const BallComponent& ball, const glm::vec3& velocity, const double& deltatime) {
		//calculate magic so old velocity matters
		glm::vec3 F{ velocity };
		glm::vec3 G(0.0, -Math::Gravity, 0.0);
		glm::vec3 N = -G;

		glm::vec3 tmpVelocity(0.f);
		F = G;
		// TODO fix velocity calculations
	   // F = ball.Mass*velocity*static_cast<float>(deltatime);
		// F = mv/t
		//calculate reflect between ball velocity and triangle
		//glm::vec3 test = glm::reflect(G, triangle.N);
		//test += velocity;
		//test *= deltatime;
		//return new velocity
		if (TriangleBallIntersect(triangle, position, ball.Radius)) {
			//LOG_INFO("Ball and triangle intersecting");
			F = (triangle.N + G) * glm::length(velocity);
			glm::vec3 tmp(triangle.N.x * triangle.N.y, triangle.N.z * triangle.N.y, (triangle.N.y * triangle.N.y) - 1);
			F = tmp * static_cast<float>(Math::Gravity);
			//F =glm::cross(triangle.N,G) * (1/ball.Mass);
			//F =(triangle.N-G) * (1/ball.Mass);
			if (glm::length(velocity) > 0.f) {
				F += velocity * static_cast<float>(deltatime);
			}
		}
		return F;
	}

	CollisionShape::CollisionShape(Shape shape)
		: shape(shape)
	{
	}

	float CollisionShape::DistanceFromPointToPlane(const glm::vec3& point, const glm::vec3& planePos, const glm::vec3& planeNormal)
	{
		return glm::dot(point - planePos, planeNormal);
	}

	bool CollisionShape::Intersect(CollisionShape* shape)
	{
		LOG_ERROR("Collision with none");
		return false;
	}

	bool CollisionShape::Intersect(AABB* a, AABB* b)
	{
		auto& aExtent = a->extent;
		auto& aPos = a->pos;

		auto& bExtent = b->extent;
		auto& bPos = b->pos;

		float xDist = abs(aPos.x - bPos.x);
		float yDist = abs(aPos.y - bPos.y);
		float zDist = abs(aPos.z - bPos.z);
		float xMinDist = aExtent.x + bExtent.x;
		float yMinDist = aExtent.y + bExtent.y;
		float zMinDist = aExtent.z + bExtent.z;

		return (xDist < xMinDist&& yDist < yMinDist&& zDist < zMinDist);
	}

	bool CollisionShape::Intersect(AABB* box, Sphere* sphere)
	{
		glm::vec3 boxPoint;
		boxPoint.x = std::clamp<float>(sphere->pos.x, box->pos.x - box->extent.x, box->pos.x + box->extent.x);
		boxPoint.y = std::clamp<float>(sphere->pos.y, box->pos.y - box->extent.y, box->pos.y + box->extent.y);
		boxPoint.z = std::clamp<float>(sphere->pos.z, box->pos.z - box->extent.z, box->pos.z + box->extent.z);
		float dist = glm::distance(boxPoint, sphere->pos);
		return (dist < sphere->radius);
	}

	bool CollisionShape::Intersect(Sphere* a, Sphere* b)
	{
		float minDist = a->radius + b->radius;
		float dist = glm::distance(a->pos, b->pos);
		return (dist < minDist);
	}

	bool CollisionShape::Intersect(AABB* aabb, Plane* plane)
	{
		float r =
			aabb->extent.x * glm::abs(plane->normal.x) +
			aabb->extent.y * glm::abs(plane->normal.y) +
			aabb->extent.z * glm::abs(plane->normal.z);
		return DistanceFromPointToPlane(aabb->pos, plane->pos, plane->normal) < -r;
	}

	bool CollisionShape::Intersect(Sphere* sphere, Plane* plane)
	{
		auto distance = DistanceFromPointToPlane(sphere->pos, plane->pos, plane->normal);
		return distance < -sphere->radius; // Ericson p.161
	}

	bool CollisionShape::Intersect(Plane* a, Plane* b)
	{
		return glm::abs(a->normal) != glm::abs(b->normal);
	}

	bool CollisionShape::Intersect(AABB* aabb, OBB* obb)
	{
		// TODO:
		return false;
	}

	bool CollisionShape::Intersect(Sphere* sphere, OBB* obb)
	{
		// TODO:
		return false;
	}

	bool CollisionShape::Intersect(Plane* plane, OBB* obb)
	{
		float r =
			obb->extent.x * glm::abs(glm::dot(plane->normal, obb->normals[0])) +
			obb->extent.y * glm::abs(glm::dot(plane->normal, obb->normals[1])) +
			obb->extent.z * glm::abs(glm::dot(plane->normal, obb->normals[2]));
		float dist = DistanceFromPointToPlane(obb->pos, plane->pos, plane->normal);
		return dist < -r;
	}

	bool CollisionShape::Intersect(OBB* a, OBB* b)
	{
		// TODO:
		const auto ab = b->pos - a->pos;
		const auto lengthAB = glm::length(ab);

		for (const auto& aNormal : a->normals)
		{

		}
		for (const auto& bNormal : b->normals)
		{

		}
		return true;
	}

	AABB::AABB()
		: CollisionShape(Shape::AABB)
	{
	}

	bool AABB::Intersect(CollisionShape* shape)
	{
		switch (shape->shape)
		{
		case Shape::AABB:
			return CollisionShape::Intersect(this, reinterpret_cast<AABB*>(shape));
		case Shape::Sphere:
			return CollisionShape::Intersect(this, reinterpret_cast<Sphere*>(shape));
		case Shape::Plane:
			return CollisionShape::Intersect(this, reinterpret_cast<Plane*>(shape));
		case Shape::Frustum:
			return shape->Intersect(this);
		case Shape::None:
			LOG_ERROR("Collision with none.");
			break;
		}

		return false;
	}

	Sphere::Sphere()
		: CollisionShape(Shape::Sphere)
	{
	}

	bool Sphere::Intersect(CollisionShape* shape)
	{
		switch (shape->shape)
		{
		case Shape::AABB:
			return CollisionShape::Intersect(reinterpret_cast<AABB*>(shape), this);
		case Shape::Sphere:
			return CollisionShape::Intersect(this, reinterpret_cast<Sphere*>(shape));
		case Shape::Plane:
			return CollisionShape::Intersect(this, reinterpret_cast<Plane*>(shape));
		case Shape::Frustum:
			return shape->Intersect(this);
		case Shape::None:
			LOG_ERROR("Collision with none.");
			break;
		}

		return false;
	}

	Plane::Plane()
		: CollisionShape(Shape::Plane)
	{
	}

	bool Plane::Intersect(CollisionShape* shape)
	{
		switch (shape->shape)
		{
		case Shape::AABB:
			return CollisionShape::Intersect(reinterpret_cast<AABB*>(shape), this);
		case Shape::Sphere:
			return CollisionShape::Intersect(reinterpret_cast<Sphere*>(shape), this);
		case Shape::Plane:
			return CollisionShape::Intersect(this, reinterpret_cast<Plane*>(shape));
		case Shape::Frustum:
			return shape->Intersect(this);
		case Shape::None:
			LOG_ERROR("Collision with none.");
			break;
		}

		return false;
	}

	OBB::OBB()
		: CollisionShape(Shape::OBB)
	{
	}

	bool OBB::Intersect(CollisionShape* shape)
	{
		// TODO:
		return false;
	}

	Frustum::Frustum(CameraComponent& camera)
		: CollisionShape(Shape::Frustum)
		, m_Camera(camera)
	{
		UpdateFrustum();
	}

	bool Frustum::Intersect(CollisionShape* shape)
	{
		for (uint32_t i = 0; i < 6; i++)
		{
			if (Faces[i].Intersect(shape))
				return false;
		}
		return true;
	}

	void Frustum::UpdateFrustum()
	{
		const float halfVSide = m_Camera.Far * tanf(m_Camera.FOV * .5f);
		const float halfHSide = halfVSide * m_Camera.Aspect;
		const glm::vec3 frontMultFar = m_Camera.Far * m_Camera.Forward;

		Faces[0].pos = m_Camera.Position + m_Camera.Near * m_Camera.Forward; // near
		Faces[0].normal = m_Camera.Forward;

		// TODO: probably wrong. copy paste from opengl project (right -> left hand coord system).
		Faces[1].pos = m_Camera.Position;  // left
		Faces[1].normal = glm::normalize(glm::cross(frontMultFar - m_Camera.Right * halfHSide, m_Camera.Up));

		Faces[2].pos = m_Camera.Position; // right
		Faces[2].normal = glm::normalize(glm::cross(m_Camera.Up, frontMultFar + m_Camera.Right * halfHSide));

		Faces[3].pos = m_Camera.Position; // bottom
		Faces[3].normal = glm::normalize(glm::cross(frontMultFar + m_Camera.Up * halfVSide, m_Camera.Right));

		Faces[4].pos = m_Camera.Position; // top
		Faces[4].normal = glm::normalize(glm::cross(m_Camera.Right, frontMultFar - m_Camera.Up * halfVSide));

		Faces[5].pos = m_Camera.Position + frontMultFar; // far
		Faces[5].normal = -m_Camera.Forward;
	}

	void Frustum::SetCamera(CameraComponent& camera)
	{
		m_Camera = camera;
	}
}