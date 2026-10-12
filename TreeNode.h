#pragma once
#include "Point.h"
#include <memory>

class TreeNode {
public:
	Point p;
	
	std::weak_ptr<TreeNode> parent;
	std::shared_ptr<TreeNode> up;
	std::shared_ptr<TreeNode> down;
	std::shared_ptr<TreeNode> left;
	std::shared_ptr<TreeNode> right;

	TreeNode(Point p);
	TreeNode(int x = 0, int y = 0);
};