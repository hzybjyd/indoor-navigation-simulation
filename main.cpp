//程序总体思路首先建立在通过按动方向键，创建不重复的节点，这是实现功能的基础
//TreeNode里面有五个节点，分别为up，down，left，right，parent，其中只有parent是没用的，属于历史产物，忽略即可
//节点之间的连接只依赖上下左右，不依赖parent，由于这是一个树形结构，或者是网状结构，只要从根节点遍历，就可以一直向下遍历，无需关心父节点
//在创建的不重复的节点的基础之上，drawMap从根节点遍历到当前操纵的节点，绘制了地图
//在创建的不重复的节点的基础之上，calculatePathDistance计算了所有的除了根节点到根节点的路径距离
//在创建的不重复的节点的基础之上，drawDistanceTable绘制了我们需要的表格
//其中drawDistanceTable的时间复杂度为n^2，水平不足，不知道是否能优化
//本程序没有很好的进行分文件处理，这也是不足之处

#include <iostream>
#include "TreeNode.h"
#include <conio.h>
#include <Windows.h>
#include <memory>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <algorithm>
#include<iomanip>
#include <string>
#include <random>
#include <fstream>
#include <ctime>
//枚举类，输入方向键会发出224以及方向对应的信号
enum {KEY_UP = 72,KEY_DOWN = 80,KEY_LEFT = 75,KEY_RIGHT = 77,ESC = 27};
void drawMap(std::shared_ptr<TreeNode> root, std::shared_ptr<TreeNode> currentPlayer);
int calculatePathDistance(std::shared_ptr<TreeNode> startNode, std::shared_ptr<TreeNode> endNode);

//生成正态分布的随机数
double randNormal() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::normal_distribution<> dist(0.0, 1.0);//标准正态分布

	return dist(gen);
}

/**
   * @brief 绘制距离分析表
   * @param root 根节点 (A点)
   * @param allNodes 所有已存在的节点列表
   * @param pointB, pointC, pointD 目标点B, C, D的坐标
*/
void drawDistanceTable(std::shared_ptr<TreeNode> root,
	const std::vector<std::shared_ptr<TreeNode>>& allNodes,
	const Point& pointB, const Point& pointC, const Point& pointD,
	std::ostream* fileOut = nullptr) {
	//为了实现取得添加噪声后找到近似点新增的结构体
	struct NodeData{
		std::shared_ptr<TreeNode> node;
		int pathA;
		double lineB, lineC, lineD;
		double diffAB, diffAC, diffAD;
		double diffAB_noise, diffAC_noise, diffAD_noise;
	};
	std::vector<NodeData> nodeDataList;//用于存储当前循环的所有节点

	//先把所有节点的数据收集起来
	for (const auto& node : allNodes) {
		if (node == root) {
			continue;
		}
		int pathA = calculatePathDistance(root, node);
		double lineB = Point::distance(node->p, pointB);
		double lineC = Point::distance(node->p, pointC);
		double lineD = Point::distance(node->p, pointD);

		double diffAB = std::abs(static_cast<double>(pathA) - lineB);
		double diffAC = std::abs(static_cast<double>(pathA) - lineC);
		double diffAD = std::abs(static_cast<double>(pathA) - lineD);

		double noise = randNormal();
		double diffAB_noise = std::abs(diffAB + noise);
		double diffAC_noise = std::abs(diffAC + noise);
		double diffAD_noise = std::abs(diffAD + noise);

		NodeData data;
		data.node = node;
		data.pathA = pathA;
		data.lineB = lineB;
		data.lineC = lineC;
		data.lineD = lineD;
		data.diffAB = diffAB;
		data.diffAC = diffAC;
		data.diffAD = diffAD;
		data.diffAB_noise = diffAB_noise;
		data.diffAC_noise = diffAC_noise;
		data.diffAD_noise = diffAD_noise;

		nodeDataList.push_back(data);
	}

	std::ostream& out = fileOut ? *fileOut : std::cout;
	out << "====================== Distance Analysis ======================" << std::endl;
	//打印表头
	out << std::left //左对齐
			  << std::setw(15) << "Node(x,y)"
			  << std::setw(12) << "Path(A)"
			  << std::setw(12) << "Line(B)"
			  << std::setw(12) << "Line(C)"
			  << std::setw(12) << "Line(D)"
			  << std::setw(12) << "Path(A)-B"
			  << std::setw(12) << "Path(A)-C"
			  << std::setw(12) << "Path(A)-D"
			  << std::setw(17) << "Path(A)-B+noise"
			  << std::setw(17) << "Path(A)-C+noise"
			  << std::setw(17) << "Path(A)-D+noise"
			  << std::setw(12) << "MinError"
			  << std::setw(15) << "BestMatch"
			  << std::setw(8) << "distance"
			  << std::endl;
	out << "------------------------------------------------------------------------------------------" <<std::endl;

	//设置浮点数输出格式
	out << std::fixed << std::setprecision(2);

	//对外圈的循环的节点i，与内圈循环的节点j进行比较，找出最佳匹配点
	for (auto& data_i : nodeDataList) {
		double minError = 1e9;
		std::shared_ptr<TreeNode> bestMatch = nullptr;

		for (auto& data_j : nodeDataList) {
			double error = 0.0;
			error += std::abs(data_i.diffAB_noise - data_j.diffAB);
			error += std::abs(data_i.diffAC_noise - data_j.diffAC);
			error += std::abs(data_i.diffAD_noise - data_j.diffAD);

			if (error < minError) {
				minError = error;
				bestMatch = data_j.node;
			}
		}

		std::string nodePos = "(" + std::to_string(data_i.node->p.x) + "," + std::to_string(data_i.node->p.y) + ")";
		std::string matchPos = "(" + std::to_string(bestMatch->p.x) + "," + std::to_string(bestMatch->p.y) + ")";

		double distance = Point::distance(data_i.node->p, bestMatch->p);//计算节点i与最佳匹配点的直线距离

		out << std::left
			<< std::setw(15) << nodePos
			<< std::setw(12) << data_i.pathA
			<< std::setw(12) << data_i.lineB
			<< std::setw(12) << data_i.lineC
			<< std::setw(12) << data_i.lineD
			<< std::setw(12) << data_i.diffAB
			<< std::setw(12) << data_i.diffAC
			<< std::setw(12) << data_i.diffAD
			<< std::setw(17) << data_i.diffAB_noise
			<< std::setw(17) << data_i.diffAC_noise
			<< std::setw(17) << data_i.diffAD_noise
			<< std::setw(12) << minError
			<< std::setw(15) << matchPos
			<< std::setw(8) <<  distance
			<< std::endl;
	}
	out << "===============================================================" << std::endl;
}

int main() {
	SetConsoleOutputCP(CP_UTF8); // 设置控制台输出为 UTF-8，支持中文显示
	std::map<std::pair<int, int>, std::shared_ptr<TreeNode>> worldMap;//用于存储不重复的节点
	auto root = std::make_shared<TreeNode>(0, 0);//根节点，我们暂时让他为A
	worldMap[{0, 0}] = root;
	Point* b = new Point(3,4);//暂时令他为B
	Point* c = new Point(6,-5);//暂时令他为C
	Point* d = new Point(-10,-2);//暂时令他为D

	std::shared_ptr<TreeNode> currentNode = root;

	std::cout << "程序，启动！" << std::endl;
	std::cout << "使用方向键探索，按ESC退出" << std::endl;

	int stepSize = 2; // 步长参数，可根据需要修改
	while (true) {
		//清屏并显示当前状态
		system("cls");
		//绘制地图
		drawMap(root, currentNode);
		//利用map的特性，把所有map里面的节点给到allNodes，保证节点的唯一性
		std::vector<std::shared_ptr<TreeNode>> allNodes;
		for (const auto& pair : worldMap) {
			allNodes.push_back(pair.second);
		}
		//绘制表格（只在终端输出）
		drawDistanceTable(root, allNodes, *b, *c, *d);

		std::cout << "========================================" << std::endl;
		std::cout << "  当前位置: (" << currentNode->p.x << ", " 
			<< currentNode->p.y << ")"<< std::endl;
		std::cout << "----------------------------------------" << std::endl;
		std::cout << "  使用方向键移动，按 ESC 键退出程序。" << std::endl;
		std::cout << "========================================" << std::endl;

		//等待键盘输入
		int key = _getch();

		//如果是ESC键，进入循环
		if (key == ESC) {
			break;
		}

		//方向键通常会返回两个字符，第一个是224
		if (key == 224) {
			int arrow = _getch();

			Point targetPos = currentNode->p;
			int direction = -1;// 0:up, 1:down, 2:left, 3:right

			// a. 计算目标坐标和方向，应用步长
			switch (arrow) {
			case KEY_UP:    targetPos.y += stepSize; direction = 0; break;
			case KEY_DOWN:  targetPos.y -= stepSize; direction = 1; break;
			case KEY_LEFT:  targetPos.x -= stepSize; direction = 2; break;
			case KEY_RIGHT: targetPos.x += stepSize; direction = 3; break;
			default: continue; // 如果不是方向键，则忽略
			}

			std::shared_ptr<TreeNode> targetNode;

			// b. 在世界地图中查找目标节点
			auto it = worldMap.find({ targetPos.x, targetPos.y });

			if (it != worldMap.end()) {
				// 如果找到了，直接使用它
				targetNode = it->second;
			}
			else {
				// 如果没找到，创建新节点并注册到世界地图
				targetNode = std::make_shared<TreeNode>(targetPos.x, targetPos.y);
				worldMap[{targetPos.x, targetPos.y}] = targetNode;
			}

			// c. 根据方向，建立双向链接
			switch (direction) {
			case 0: // UP
				currentNode->up = targetNode;
				targetNode->down = currentNode;
				break;
			case 1: // DOWN
				currentNode->down = targetNode;
				targetNode->up = currentNode;
				break;
			case 2: // LEFT
				currentNode->left = targetNode;
				targetNode->right = currentNode;
				break;
			case 3: // RIGHT
				currentNode->right = targetNode;
				targetNode->left = currentNode;
				break;
			}

			// d. 移动到目标节点
			currentNode = targetNode;
		}
	}

	// 程序结束时只输出最后一次表格到文件
	std::vector<std::shared_ptr<TreeNode>> finalNodes;
	for (const auto& pair : worldMap) {
		finalNodes.push_back(pair.second);
	}
	char filename[128];
	std::time_t t = std::time(nullptr);
	std::tm tm;
#ifdef _WIN32
	localtime_s(&tm, &t);
#else
	tm = *std::localtime(&t);
#endif
	std::strftime(filename, sizeof(filename), "output/distance_%Y%m%d_%H%M%S.txt", &tm);
	std::ofstream fout(filename, std::ios::out | std::ios::trunc);
	if (!fout) {
		std::cerr << "无法创建输出文件: " << filename << std::endl;
	} else {
		drawDistanceTable(root, finalNodes, *b, *c, *d, &fout);
		fout.close();
	}
	delete b;
	delete c;
	delete d;
	std::cout << "\n系统已退出" << std::endl;
	return 0;
}

//绘制地图
void drawMap(std::shared_ptr<TreeNode> root, std::shared_ptr<TreeNode> currentPlayer) {
	if (!root) {
		return;
	}

	//遍历所有节点，找出地图边界
	std::vector<std::shared_ptr<TreeNode>> allNodes;
	std::set<TreeNode*> visited;//用于存储不重复的节点，然后再给到allNodes，保证allNodes里面不重复
	std::queue<std::shared_ptr<TreeNode>> q;//对于我们找到的每一个新的节点，都要先给到q队列，q队列出列后再给到allNodes

	q.push(root);
	visited.insert(root.get());

	int minX = root->p.x;
	int maxX = root->p.x;
	int minY = root->p.y;
	int maxY = root->p.y;

	while (!q.empty()) {
		auto node = q.front();
		q.pop();
		allNodes.push_back(node);

		//找到地图的边界
		minX = (std::min)(minX, node->p.x);
		maxX = (std::max)(maxX, node->p.x);
		minY = (std::min)(minY, node->p.y);
		maxY = (std::max)(maxY, node->p.y);

		//将邻居节点加入队列，如果节点存在，且map里面没找到，说明这是一个新的节点
		if (node->up && visited.find(node->up.get()) == visited.end()) {
			q.push(node->up);
			visited.insert(node->up.get());
		}
		if (node->down && visited.find(node->down.get()) == visited.end()) {
			q.push(node->down);
			visited.insert(node->down.get());
		}
		if (node->left && visited.find(node->left.get()) == visited.end()) {
			q.push(node->left);
			visited.insert(node->left.get());
		}
		if (node->right && visited.find(node->right.get()) == visited.end()) {
			q.push(node->right);
			visited.insert(node->right.get());
		}
	}

	//创建一个用于快速查找的显示地图
	std::map<std::pair<int, int>, char> displayMap;
	for (const auto& node : allNodes) {
		displayMap[{node->p.x, node->p.y}] = '.';//默认为普通节点
	}
	displayMap[{root->p.x, root->p.y}] = 'R'; // 根节点
	displayMap[{currentPlayer->p.x, currentPlayer->p.y}] = 'P'; // 玩家位置

	// 3. 绘制地图
	std::cout << "================ Map ================" << std::endl;
	for (int y = maxY; y >= minY; --y) {
		std::cout << "  "; // 左边距
		for (int x = minX; x <= maxX; ++x) {
			if (displayMap.count({ x, y })) {
				std::cout << displayMap[{x, y}] << " ";
			}
			else {
				std::cout << "  "; // 未探索区域
			}
		}
		std::cout << std::endl;
	}
	std::cout << "=====================================" << std::endl;
	std::cout << " P: Player | R: Root | .: Visited" << std::endl;
	std::cout << "-------------------------------------" << std::endl;
}

/**
   * @brief 使用广度优先搜索(BFS)计算两个节点之间的最短路径距离
   * @param startNode 起始节点
   * @param endNode 目标节点
   * @return 路径距离。如果无法到达，返回-1。
*/
int calculatePathDistance(std::shared_ptr<TreeNode> startNode, std::shared_ptr<TreeNode> endNode) {
	if (!startNode || !endNode) {
		return -1;
	}
	if (startNode == endNode) {
		return 0;
	}

	std::queue < std::pair<std::shared_ptr<TreeNode>, int>> q;//存储（节点，距离）
	std::set<TreeNode*> visited;

	q.push({ startNode,0 });
	visited.insert(startNode.get());

	while (!q.empty()) {
		auto current = q.front();
		q.pop();
		std::shared_ptr<TreeNode> currentNode = current.first;
		int currentDist = current.second;

		//检查四个方向的邻居
		std::vector<std::shared_ptr<TreeNode>> neighbors = { currentNode->up,
		currentNode->down,currentNode->left,currentNode->right };
		for (const auto& neighbor : neighbors) {
			if (neighbor) {
				//如果我们终于遍历到了目标节点，那就只需要在当前的节点的距离加上1就是目标节点距离起点的路径距离了
				if (neighbor == endNode) {
					return currentDist + 1;
				}
				//如果我们找到了这个节点，但是这个节点还没在集合上，我们就要把这个节点加入集合，然后把我们找到的新节点的距离加一
				if (visited.find(neighbor.get()) == visited.end()) {
					visited.insert(neighbor.get());
					q.push({ neighbor,currentDist + 1 });
				}
			}
		}
	}

	return -1;
}