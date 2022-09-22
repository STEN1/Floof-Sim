#pragma once

#include <vector>
#include "Physics.h"
#include "entt/entt.hpp"

namespace FLOOF {
	class Octree {
	public:
		using CollisionObject = std::pair<entt::entity, CollisionShape*>;

		Octree(const AABB& aabb);
		void Insert(const CollisionObject& object);
		void FindIntersectingObjects(const CollisionObject& object, std::vector<CollisionObject>& outVec);
		void Divide();
		void GetActiveLeafNodes(std::vector<Octree*>& outVec);
		void GetAllNodes(std::vector<Octree*>& outVec);
		AABB GetAABB() { return m_AABB; }
	private:
		AABB m_AABB;
		bool IsLeaf();
		std::vector<std::unique_ptr<Octree>> m_ChildNodes;
		std::vector<CollisionObject> m_CollisionObjects;
		inline static uint32_t s_MaxObjects = 20;
		inline static float s_MinExtent = 0.1f;
	};
}