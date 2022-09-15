#pragma once

#include "Math.h"
#include "Components.h"

namespace FLOOF{
    class Physics {
    public:
        Physics() = delete;

    static bool TriangleBallIntersect(const Triangle &triangle, const glm::vec3 &position, const float &radius);
    static glm::vec3 GetReflectVelocity(const glm::vec3 & velocity, const glm::vec3 & reflectionangle);
    static glm::vec3 GetReflectionAngle(const glm::vec3 & m, const glm::vec3 & n);
    static glm::vec3 GetAccelerationVector(const Triangle& triangle);
    static void ElasticCollision(glm::vec3 p1, glm::vec3 p2, glm::vec3 &v1, glm::vec3 &v2);

    };

	class AABB;
	class Sphere;
	class Plane;
	class OBB;
	class Frustum;

	class Camera;

	class CollisionShape
	{
	public:
		enum class Shape : uint8_t
		{
			None = 0,
			AABB,
			Sphere,
			Plane,
			OBB,
			Frustum
		};
		virtual bool Intersect(CollisionShape* shape);

		static bool Intersect(AABB* a, AABB* b);

		static bool Intersect(AABB* box, Sphere* sphere);
		static bool Intersect(Sphere* a, Sphere* b);

		static bool Intersect(AABB* aabb, Plane* plane);
		static bool Intersect(Sphere* sphere, Plane* plane);
		static bool Intersect(Plane* a, Plane* b);

		static bool Intersect(AABB* aabb, OBB* obb);
		static bool Intersect(Sphere* sphere, OBB* obb);
		static bool Intersect(Plane* plane, OBB* obb);
		static bool Intersect(OBB* a, OBB* b);

		const Shape shape;
		glm::vec3 pos{};
	protected:
		CollisionShape(Shape shape);
		static float DistanceFromPointToPlane(const glm::vec3& point, const glm::vec3& planePos, const glm::vec3& planeNormal);
	};

	class AABB : public CollisionShape
	{
	public:
		AABB();
		virtual bool Intersect(CollisionShape* shape) override;
		glm::vec3 extent{ 0.5f }; // half extent.
	};

	class Sphere : public CollisionShape
	{
	public:
		Sphere();
		virtual bool Intersect(CollisionShape* shape) override;
		float radius{ 0.5f };
	};

	class Plane : public CollisionShape
	{
	public:
		Plane();
		virtual bool Intersect(CollisionShape* shape) override;
		glm::vec3 normal{ 0.f, 1.f, 0.f };
	};

	class OBB : public CollisionShape
	{
	public:
		OBB();
		virtual bool Intersect(CollisionShape* shape) override;
		glm::vec3 extent{ 0.5f }; // half extent.
		glm::vec3 normals[3];
	};

	class Frustum : public CollisionShape
	{
	public:
		Frustum(CameraComponent& camera);
		virtual bool Intersect(CollisionShape* shape) override;
		void UpdateFrustum();
		void SetCamera(CameraComponent& camera);
	private:
		CameraComponent& m_Camera;
		Plane Faces[6]; // near left right bottom top far.
	};
}
