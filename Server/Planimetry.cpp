//
// Created by dpons on 5/19/19.
//

#include "Planimetry.h"

Planimetry::Planimetry() {
    log = new Logger("Planimetry", true);

}

/**
 * Add new node to planimetry
 * @param ID
 * @param cpuPower
 * @param x
 * @param z
 * @param theta
 * @param toAdd
 * @param up
 * @param bottom
 * @param left
 * @param right
 */
void Planimetry::addNode(int ID, int cpuPower, int x, int z, int theta, Instruction *toAdd, int up, int bottom, int left, int right) {
    for (Node * node : planimetry) {
        if (node->ID == ID && node->thisNode == nullptr) {
            node->thisNode = toAdd;
            node->x = x;
            node->z = z;
            node->theta = theta;
            node->cpuPower = cpuPower;
            node->up = getNode(up);
            if (node->up != nullptr) node->up->bottom = node;
            node->bottom = getNode(bottom);
            if (node->bottom != nullptr) node->bottom->up = node;
            node->left = getNode(left);
            if (node->left != nullptr) node->left->right = node;
            node->right = getNode(right);
            if (node->right != nullptr) node->right->left = node;
            return;
        }
    }

    Node * newNode = (Node *) malloc(sizeof(Node));
    if (newNode == nullptr) return;

    newNode->ID = ID;
    newNode->thisNode = toAdd;
    newNode->cpuPower = cpuPower;
    newNode->x = x;
    newNode->z = z;
    newNode->theta = theta;
    planimetry.push_back(newNode);

    // If no adjacent exists, create new nodes with null SNode
    if (up != -1 && getNode(up)== nullptr) addNode(up, cpuPower, -1, -1, -1, nullptr, -1, ID, -1, -1);
    if (bottom != -1 && getNode(bottom) == nullptr) addNode(bottom, cpuPower, -1, -1, -1, nullptr, ID, -1, -1, -1);
    if (left != -1 && getNode(left) == nullptr) addNode(left, cpuPower, -1, -1, -1, nullptr, -1, -1, -1, ID);
    if (right != -1 && getNode(right) == nullptr) addNode(right, cpuPower, -1, -1, -1, nullptr, -1, -1, ID, -1);

    newNode->up = getNode(up);
    if (newNode->up != nullptr) newNode->up->bottom = newNode;
    newNode->bottom = getNode(bottom);
    if (newNode->bottom != nullptr) newNode->bottom->up = newNode;
    newNode->left = getNode(left);
    if (newNode->left != nullptr) newNode->left->right = newNode;
    newNode->right = getNode(right);
    if (newNode->right != nullptr) newNode->right->left = newNode;
}

/**
 * @param toFind
 * @return node from defined SNode ref
 */
Node *Planimetry::getNode(int toFind) {
    if (toFind == -1) return nullptr;
    for (Node * node : planimetry)
        if (node->ID == toFind) return node;
    return nullptr;
}


/**
 * @param toFind
 * @return node from defined SNode ref
 */
Node *Planimetry::getNode(Instruction *toFind) {
    for (Node * node : planimetry)
        if (node->thisNode == toFind) return node;
    return nullptr;
}

/**
 * @param toFind socket of node to find
 * @return node from defined SNode ref
 */
Node *Planimetry::getNodeBySocket(int toFind) {
    for (Node * node : planimetry)
        if (node->thisNode != nullptr &&
            node->thisNode->getNodeSocket() == toFind) return node;
    return nullptr;
}

/**
 * @param toFree node to free
 */
void Planimetry::freeNode(Instruction *toFree) {
    Node * nodeToFree = getNode(toFree);

    if (toFree != nullptr) free(toFree);
}

/**
 * Default destructor
 */
Planimetry::~Planimetry() {
    for (Node * node : planimetry) free(node);
    delete log;
}
