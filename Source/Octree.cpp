#include "Octree.h"

namespace FLOOF {
	Octree::Octree(const AABB& aabb) 
		: m_AABB(aabb) {
	}

	void Octree::Insert(CollisionObject object) {
		if (!object.second->Intersect(&m_AABB))
			return;
		
		if (IsLeaf()) {
			// intersecting with node and node is leaf. insert.

			m_CollisionObjects.push_back(object);

			if (m_CollisionObjects.size() > s_MaxObjects && m_AABB.extent.x > s_MinExtent) {
				// to many objects in node and extent is larger than min extent.
				Divide();
				for (auto& node : m_ChildNodes) {
					for (auto& obj : m_CollisionObjects) {
						node->Insert(obj);
					}
				}
				m_CollisionObjects.clear();
			}
		}
	}

	void Octree::FindIntersectingObjects(CollisionObject object, std::vector<CollisionObject>& outVec) {
		for (auto& node : m_ChildNodes) {
			node->FindIntersectingObjects(object, outVec);
		}

		if (IsLeaf()) {
			for (auto& obj : m_CollisionObjects) {
				if (obj.second->Intersect(object.second)) {
					auto it = std::find(outVec.begin(), outVec.end(), obj);
					if (it == outVec.end()) {
						outVec.push_back(obj);
					}
				}
			}
		}
	}

	bool Octree::IsLeaf() {
		if (m_ChildNodes.empty())
			return true;
		return false;
	}
	
	void Octree::Divide() {
		AABB a = m_AABB;
		a.extent /= 2.f;
		a.pos.x -= a.extent.x;
		a.pos.z -= a.extent.z;
		a.pos.y -= a.extent.y;
		AABB b = m_AABB;
		b.extent /= 2.f;
		b.pos.x -= b.extent.x;
		b.pos.z += b.extent.z;
		b.pos.y -= b.extent.y;
		AABB c = m_AABB;
		c.extent /= 2.f;
		c.pos.x += c.extent.x;
		c.pos.z += c.extent.z;
		c.pos.y -= c.extent.y;
		AABB d = m_AABB;
		d.extent /= 2.f;
		d.pos.x += d.extent.x;
		d.pos.z -= d.extent.z;
		d.pos.y -= d.extent.y;

		AABB e = m_AABB;
		e.extent /= 2.f;
		e.pos.x -= e.extent.x;
		e.pos.z -= e.extent.z;
		e.pos.y += e.extent.y;
		AABB f = m_AABB;
		f.extent /= 2.f;
		f.pos.x -= f.extent.x;
		f.pos.z += f.extent.z;
		f.pos.y += f.extent.y;
		AABB g = m_AABB;
		g.extent /= 2.f;
		g.pos.x += g.extent.x;
		g.pos.z += g.extent.z;
		g.pos.y += g.extent.y;
		AABB h = m_AABB;
		h.extent /= 2.f;
		h.pos.x += h.extent.x;
		h.pos.z -= h.extent.z;
		h.pos.y += h.extent.y;

		m_ChildNodes.reserve(8);
		m_ChildNodes.emplace_back(std::make_unique<Octree>(a));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(b));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(c));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(d));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(e));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(f));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(g));
		m_ChildNodes.emplace_back(std::make_unique<Octree>(h));
	}

	void Octree::GetLeafNodes(std::vector<Octree*> outVec) {
		for (auto& node : m_ChildNodes) {
			node->GetLeafNodes(outVec);
		}

		if (IsLeaf()) {
			outVec.push_back(this);
		}
	}
}