#include "rbtree.h"
#include <stdlib.h>
#include <malloc/malloc.h>
#include <stdio.h>

void left_rotate(rbtree *t, node_t *node);
void right_rotate(rbtree *t, node_t *node);
void insert_fixup(rbtree *t, node_t *node);
void delete_node(node_t *node);
void transplant(rbtree *t, node_t *replaced_node, node_t *replacing_node);
node_t *minimum(node_t *node);
void delete_fixup(rbtree *t, node_t *node);
node_t *nil_or_node(node_t *node);

node_t NIL = {RBTREE_BLACK, 0, NULL, NULL, NULL};

rbtree *new_rbtree(void) {
  rbtree *p = (rbtree *)calloc(1, sizeof(rbtree));
  return p;
}

void delete_rbtree(rbtree *t) {
  // TODO: reclaim the tree nodes's memory
  node_t *current = t->root;
  if (current == NULL)
    return;
  else{
    node_t *left_node = current->left;
    node_t *right_node = current->right;
    free(current);
    delete_node(left_node);
    delete_node(right_node);
  }

  free(t);
}

node_t *rbtree_insert(rbtree *t, const key_t key) {
  // TODO: implement insert
  if (t->root == NULL){
    t->root = (node_t *)calloc(1, sizeof(node_t));

    t->root->key = key;
    t->root->parent = NULL;
    t->root->left = NULL;
    t->root->right = NULL;
    t->root->color = RBTREE_BLACK;

    return t->root;
  }

  node_t *current = t->root;
  node_t *inserted_node = NULL;

  while(1) {
    if (current->key < key) {
      if (current->right == NULL) {
        current->right = (node_t *)calloc(1, sizeof(node_t));
        current->right->key = key;
        current->right->parent = current;
        current->right->left = NULL;
        current->right->right = NULL;
        current->right->color = RBTREE_RED;
        inserted_node = current->right;
        break;
      } else {
        current = current->right;
      }
    } else {
      if (current->left == NULL) {
        current->left = (node_t *)calloc(1, sizeof(node_t));
        current->left->key = key;
        current->left->parent = current;
        current->left->left = NULL;
        current->left->right = NULL;
        current->left->color = RBTREE_RED;
        inserted_node = current->left;
        break;
      } else {
        current = current->left;
      }
    }
  }

  insert_fixup(t, inserted_node);

  return t->root;
}

node_t *rbtree_find(const rbtree *t, const key_t key) {
  // TODO: implement find
  node_t *current = t->root;
  while (current != NULL && current->key != key) {
    if (key > current->key)
      current = current->right;
    else
      current = current->left;
  }

  return current;
}

node_t *rbtree_min(const rbtree *t) {
  // TODO: implement find
  node_t *current = t->root;
  if(current == NULL)
    return current;
  while (current->left != NULL)
    current = current->left;

  return current;
}

node_t *rbtree_max(const rbtree *t) {
  // TODO: implement find
  node_t *current = t->root;
  if(current == NULL)
    return current;
  while (current->right != NULL)
    current = current->right;

  return current;
}

int rbtree_erase(rbtree *t, node_t *p) {
  // TODO: implement erase
  node_t *removed_node = p;
  color_t original_color = removed_node->color;
  node_t *inserting_node = NULL;

  if (p->left == NULL) {
    inserting_node = nil_or_node(p->right);
    transplant(t, p, inserting_node);
  } else if (p->right == NULL) {
    inserting_node = nil_or_node(p->left);
    transplant(t, p, inserting_node);
  } else {
    removed_node = minimum(p->right);
    original_color = removed_node->color;
    inserting_node = nil_or_node(removed_node->right);

    if (removed_node->parent == p)
      inserting_node->parent = removed_node;
    else {
      transplant(t, removed_node, inserting_node);
      removed_node->right = p->right;
      removed_node->right->parent = removed_node;
    }
    transplant(t, p, removed_node);
    removed_node->left = p->left;
    removed_node->left->parent = removed_node;
    removed_node->color = original_color;
  }

  if (original_color == RBTREE_BLACK)
    delete_fixup(t, inserting_node);

  if(t->root == &NIL) {
    t->root = NULL;
  } else if (NIL.parent != NULL) {
    if (NIL.parent->left == &NIL) {
      NIL.parent->left = NULL;
    } else if (NIL.parent->right == &NIL) {
      NIL.parent->right = NULL;
    }
    NIL.parent = NULL;
  }

  free(p);
  p = NULL;

  if (t->root != NULL) t->root->color = RBTREE_BLACK;
  return 0;
}

int rbtree_to_array(const rbtree *t, key_t *arr, const size_t n) {
  // TODO: implement to_array
  return 0;
}

void left_rotate(rbtree *t, node_t *node) {
  node_t *next_node = node->right;
  node->right = next_node->left;
  if (next_node->left != NULL)
    next_node->left->parent = node;
  next_node->parent = node->parent;
  if (node->parent == NULL)
    t->root = next_node;
  else if (node == node->parent->left)
    node->parent->left = next_node;
  else
    node->parent->right = next_node;
  next_node->left = node;
  node->parent = next_node;
}

void right_rotate(rbtree *t, node_t *node) {
  node_t *next_node = node->left;
  node->left = next_node->right;
  if (next_node->right != NULL)
    next_node->right->parent = node;
  next_node->parent = node->parent;
  if (node->parent == NULL)
    t->root = next_node;
  else if (node == node->parent->left)
    node->parent->left = next_node;
  else
    node->parent->right = next_node;
  next_node->right = node;
  node->parent = next_node;
}

void insert_fixup(rbtree *t, node_t *node) {
  while (node->parent != NULL && node->parent->color == RBTREE_RED) {
    // node 부모가 할아버지의 왼쪽 자식인 경우
    if (node->parent == node->parent->parent->left){
      node_t *uncle_node = node->parent->parent->right;
      // 1. 삼촌 색깔이 빨간색 일때
      if (uncle_node != NULL && uncle_node->color == RBTREE_RED){
        node->parent->color = RBTREE_BLACK;
        uncle_node->color = RBTREE_BLACK;
        node->parent->parent->color = RBTREE_RED;
        node = node->parent->parent;
      // 2. 삼촌 색깔이 검정색 일때
      } else {
        // 1) 내가 부모의 오른쪽일때
        if (node == node->parent->right) {
          node = node->parent;
          left_rotate(t, node);
        }
        // 2) 내가 부모의 왼쪽일때
        node->parent->color = RBTREE_BLACK;
        node->parent->parent->color = RBTREE_RED;
        right_rotate(t, node->parent->parent);
      }
    // node 부모가 할아버지의 오른쪽 자식인 경우
    } else {
      node_t *uncle_node = node->parent->parent->left;
      // 1. 삼촌 색깔이 빨간색 일때
      if (uncle_node != NULL && uncle_node->color == RBTREE_RED) {
        node->parent->color = RBTREE_BLACK;
        uncle_node->color = RBTREE_BLACK;
        node->parent->parent->color = RBTREE_RED;
        node = node->parent->parent;
      // 2. 삼촌 색깔이 검정색때일때
      } else {
        // 1) 내가 부모의 왼쪽 일때
        if (node == node->parent->left) {
          node= node->parent;
          right_rotate(t, node);
        }
        // 2) 내가 부모 오른쪽 일때
        node->parent->color = RBTREE_BLACK;
        node->parent->parent->color = RBTREE_RED;
        left_rotate(t, node->parent->parent);
      }
    }
  }
  t->root->color = RBTREE_BLACK;
}

void delete_node(node_t *node) {
  node_t *current = node;
  if (current == NULL)
    return;
  else {
    node_t *left_node = current->left;
    node_t *right_node = current->right;
    free(current);
    delete_node(left_node);
    delete_node(right_node);
  }
}

void transplant(rbtree *t, node_t *replaced_node, node_t *replacing_node) {
  if (replaced_node->parent == NULL)
    t->root = replacing_node;
  else if (replaced_node == replaced_node->parent->left)
    replaced_node->parent->left = replacing_node;
  else 
    replaced_node->parent->right = replacing_node;
  replacing_node->parent = replaced_node->parent;
}

node_t* minimum(node_t *node) {
    node_t *current = node;
    if (current == NULL)
        return current;
    while (current->left != NULL)
        current = current->left;
    return current;
}

void delete_fixup(rbtree *t, node_t *node) {
  while (node != t->root && node->color == RBTREE_BLACK) {
    // 1) x가 x 부모의 왼쪽 자식일때
    if (node == node->parent->left) {
      node_t *sibling = node->parent->right;
      // 1. 형제 노드의 색깔이 빨간색 일때
      if (sibling->color == RBTREE_RED) {
        sibling->color = RBTREE_BLACK;
        node->parent->color = RBTREE_RED;
        left_rotate(t, node->parent);
        sibling = node->parent->right;
      }

      // 2. 형제 노드의 색깔이 검정색이고 자식 노드들 모두 검정색일 때
      node_t *sibling_left = nil_or_node(sibling->left);
      node_t *sibling_right = nil_or_node(sibling->right);

      if (sibling_left->color == RBTREE_BLACK && sibling_right->color == RBTREE_BLACK) {
        sibling->color = RBTREE_RED;
        node = node->parent;
      } else {
        // 3. 형제 노드의 색깔이 검정색이고 오른쪽 자식 노드만 검정색일 떄
        if (sibling_right->color == RBTREE_BLACK) {
          sibling_left->color = RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          right_rotate(t, sibling);
          sibling = node->parent->right;
        }
        // 4. 형제 노드의 색깔이 검정색이고 왼쪽 자식 노드만 검정색일 때
        sibling->color = node->parent->color;
        node->parent->color = RBTREE_BLACK;
        sibling_right->color = RBTREE_BLACK;
        left_rotate(t, node->parent);
        node = t->root;
      }
    // 2) x가 x 부모의 오른쪽 자식일때
    } else {
      node_t *sibling = node->parent->left;
      // 1. 형제 노드의 색깔이 빨간색 일때
      if ( sibling->color == RBTREE_RED) {
        sibling->color = RBTREE_BLACK;
        node->parent->color = RBTREE_RED;
        right_rotate(t, node->parent);
        sibling = node->parent->left;
      }

      node_t *sibling_left = nil_or_node(sibling->left);
      node_t *sibling_right = nil_or_node(sibling->right);

      // 2. 형제 노드의 색깔이 검정색이고 자식 노드들 모두 검정색일 때
      if (sibling_left->color == RBTREE_BLACK && sibling_right->color == RBTREE_BLACK) {
        sibling->color = RBTREE_RED;
        node = node->parent;
      } else {
        // 3. 형제 노드의 색깔이 검정색이고 왼쪽 자식 노드만 검정색일 때
        if (sibling_left->color == RBTREE_BLACK) {
          sibling_right->color = RBTREE_BLACK;
          sibling->color = RBTREE_RED;
          left_rotate(t, sibling);
          sibling = node->parent->left;
        }
        // 4. 형제 노드의 색깔이 검정색이고 오른쪽 자식 노드만 검정색일 때
        sibling->color = node->parent->color;
        node->parent->color = RBTREE_BLACK;
        sibling_left->color = RBTREE_BLACK;
        right_rotate(t, node->parent);
        node = t->root;
      }
    }
  }

  node->color = RBTREE_BLACK;
}

node_t *nil_or_node(node_t *node) {
  if (node == NULL) {
    return &NIL;
  } else {
    return node;
  }
}