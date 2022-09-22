#pragma once

#include <vector>
#include "Components.h"
#include "Physics.h"
#include "entt/entt.hpp"

namespace FLOOF {
	class Octree {
	public:
		struct CollisionObject {
			CollisionObject(CollisionShape* shape, TransformComponent& transform, VelocityComponent& velocity, BallComponent& ball) 
				: Shape(shape), Transform(transform), Velocity(velocity), Ball(ball)
			{}
			CollisionShape* Shape;
			TransformComponent& Transform;
			VelocityComponent& Velocity;
			BallComponent& Ball;

			bool operator == (const CollisionObject& other) const {
				return Shape == other.Shape;
			}
		};

		Octree(const AABB& aabb);
		void Insert(const CollisionObject& object);
		void FindIntersectingObjects(const CollisionObject& object, std::vector<CollisionObject>& outVec);
		void GetCollisionPairs(std::vector<std::pair<CollisionObject, CollisionObject>>& outVec);
		void Divide();
		void GetActiveLeafNodes(std::vector<Octree*>& outVec);
		void GetAllNodes(std::vector<Octree*>& outVec);
		AABB GetAABB() { return m_AABB; }
	private:
		AABB m_AABB;
		bool m_IsActive = false;
		bool IsLeaf();
		std::vector<std::unique_ptr<Octree>> m_ChildNodes;
		std::vector<CollisionObject> m_CollisionObjects;
		inline static uint32_t s_MaxObjects = 20;
		inline static float s_MinExtent = 0.025f;
	};
}