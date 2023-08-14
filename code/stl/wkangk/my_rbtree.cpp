/***************************************************************
 * @copyright  Copyright © 2023 wkangk.
 * @file       my_rbtree.h
 * @author     wkangk <wangkangchn@163.com>
 * @version    v1.0
 * @brief      红黑树
 * @date       2023-08-09 20:22
 **************************************************************/
#include <memory>
#include <stack>
#include <iostream>



typedef bool Color;     /* enum 是整数!!! bool 只有一个字节!!! 可以可以, 神灵在细节 */
const bool rb_tree_red = false; 
const bool rb_tree_black = true; 

/* 红黑树基本的节点, 仅考虑指针域 */
// struct RBTreeNodeBase
// {
//     virtual ~RBTreeNodeBase() = default;

//     Color color_{rb_tree_red};      /* 默认新节点是红色, 满足规则 (4) */
//     std::shared_ptr<RBTreeNodeBase> parent_{nullptr};
//     std::shared_ptr<RBTreeNodeBase> left_{nullptr};
//     std::shared_ptr<RBTreeNodeBase> right_{nullptr};
// };


template <typename T>
struct RBTreeNode
{   
    typedef T   value_type;
    RBTreeNode(const value_type& val) : 
        data_(val)
    {
    }

    value_type  data_;
    Color color_{rb_tree_red};      /* 默认新节点是红色, 满足规则 (4) */
    std::shared_ptr<RBTreeNode<value_type>> parent_{nullptr};
    std::shared_ptr<RBTreeNode<value_type>> left_{nullptr};
    std::shared_ptr<RBTreeNode<value_type>> right_{nullptr};
};


template <typename T>
class RBTree
{
    typedef T value_type;
    typedef RBTreeNode<value_type>  TreeNode;

public:
    RBTree() :
        header_(new TreeNode(0))
    {
    }

public:
    void insert(const value_type& value)
    {
        if (!header_->parent_) {    /* 第一个节点 */
            header_->color_ = rb_tree_black;
            header_->parent_ = createNode(value);
            header_->parent_->color_ = rb_tree_black;    /* 根节点为黑色 */
            header_->parent_->parent_ = header_;
            return;
        }

        /* 按照二叉搜索树进行插入 */
        auto node = header_->parent_;

        while (true) {
            // if (value <= node->data_) {     /* 小于插入左边 */
            if (value <= node->data_) {     /* 小于插入左边 */
                if (node->left_ == nullptr) {
                    node->left_ = createNode(value); /* 左节点空, 插入 */
                    node->left_->parent_ = node;
                    adjust(node->left_);
                    goto end;
                } else {
                    node = node->left_;
                }
            } else {
                if (node->right_ == nullptr) {
                    node->right_ = createNode(value); /* 右节点空, 插入 */
                    node->right_->parent_ = node;
                    adjust(node->right_);
                    goto end;
                } else {
                    node = node->right_;
                }
            }
        }
end:
        adjustHeader();
    }

    void show() const
    {
        std::stack<std::shared_ptr<TreeNode>> my_stack;
        auto node = header_->parent_;
        
        while (node || !my_stack.empty()) {
            /* 找到最左 */
            while (node) {
                my_stack.push(node);
                node = node->left_; /* 直到找到最左 */
            }

            /* 到这里到了最左, 输出 */
            node = my_stack.top();
            std::cout << node->data_ << " ";
            my_stack.pop(); /* 已遍历的节点, 不会再有机会重新遍历左子树了, 因为显式指定到了右子树 */

            node = node->right_;
        }

        std::cout << std::endl;
    }

private:
    /**
     *     调整节点颜色
     */
    void adjust(std::shared_ptr<TreeNode> node)
    {
        bool s_is_left = true;

        auto x = node;
        auto p = node->parent_;

        if (p->color_ == rb_tree_black) {   /* 不违反规则, 无序调整 */
            return;
        }
        
        auto g = p->parent_;
        
        std::shared_ptr<TreeNode> s;
        if (g->left_ == p) {
            s = g->right_;
            s_is_left = false;
        } else {
            s = g->left_;
        }

        auto gg = g->parent_;

        /* 状况 1: S 为黑且 X 为外侧插入(外侧为 x 与 s 不同侧) */
        if ((!s || s->color_ == rb_tree_black) && is_outside(x, g, s)) {
            
            /* s 在左边那要左旋, s 在右边要右旋 */
            s_is_left ? l_rotate(p, g) : r_rotate(p, g);

            p->color_ = rb_tree_black;
            g->color_ = rb_tree_red;

        /* 状况 2: S 为黑且 X 为内侧插入 (内侧为 x 与 s 同侧) */
        } else if ((!s || s->color_ == rb_tree_black) && !is_outside(x, g, s)) {
            /* 先旋转 p x */
            s_is_left ? rl_rotate(x, p, g) : lr_rotate(x, p, g);

        } else if ((s && s->color_ == rb_tree_red)) {
            /* 状况 3: S 为红且 X 为外侧插入, GG 为黑 
            S 为红不需要转, 仅需要调整节点！！！
            */
            p->color_ = rb_tree_black;
            s->color_ = rb_tree_black;

            if (g != header_->parent_) { // 根结点不要调整
                g->color_ = rb_tree_red;
            }

            /* 状况 4: s 为红且 x 为外侧插入, gg 为红  */
            if (gg->color_ == rb_tree_red) {
                adjust(g);
            }
        } 
    }

    bool is_outside(std::shared_ptr<TreeNode> x, std::shared_ptr<TreeNode> g, std::shared_ptr<TreeNode> s)
    {   
        /* s 与 x 不同侧为 外 */
        if (x->parent_->left_ == x && g->right_ == s) {
            return true;
        } else if (x->parent_->right_ == x && g->left_ == s) {
            return true;
        } else {
            return false;
        }
    }

    void r_rotate(std::shared_ptr<TreeNode> p, std::shared_ptr<TreeNode> g)
    {
        adjust_parent(p, g);

        g->left_ = p->right_;
        p->right_ = g;
    }

    void l_rotate(std::shared_ptr<TreeNode> p, std::shared_ptr<TreeNode> g)
    {
        adjust_parent(p, g);

        g->right_ = p->left_;
        p->left_ = g;
    }

    void lr_rotate(std::shared_ptr<TreeNode> x, std::shared_ptr<TreeNode> p, std::shared_ptr<TreeNode> g)
    {
        l_rotate(x, p);
        g->color_ = rb_tree_red;
        x->color_ = rb_tree_black; 
        r_rotate(x, g);
    }

    void rl_rotate(std::shared_ptr<TreeNode> x, std::shared_ptr<TreeNode> p, std::shared_ptr<TreeNode> g)
    {
        r_rotate(x, p);
        g->color_ = rb_tree_red;
        x->color_ = rb_tree_black; 
        l_rotate(x, g);
    }

    /**
     * @param [in]  p   原 子
     * @param [in]  g   p 的父 
     * @return     无
     */
    void adjust_parent(std::shared_ptr<TreeNode> p, std::shared_ptr<TreeNode> g)
    {
        if (g->parent_ == header_) {    /* 找到了根节点 */
            header_->parent_ = p;
        } else {
            if (g->parent_->left_ == g) {   /* 先调整 g 的父 */
                g->parent_->left_ = p;
            } else {
                g->parent_->right_ = p;
            }
        }

        p->parent_ = g->parent_;
        g->parent_ = p;
    }

    /**
     *     调整 head 的最左和最右节点
     */
    void adjustHeader()
    {
        header_->left_ = leftmost();
        header_->right_ = rightmost();
    }

    std::shared_ptr<TreeNode> leftmost() const 
    {
        auto node = header_->parent_;

        while (node && node->left_) {
            node = node->left_;
        }

        return node;
    }

    std::shared_ptr<TreeNode> rightmost() const 
    {
        auto node = header_->parent_;

        while (node && node->right_) {
            node = node->right_;
        }

        return node;
    }

    std::shared_ptr<TreeNode> createNode(const value_type& value) const
    {
        return std::make_shared<TreeNode>(value);
    }

private:
    std::shared_ptr<TreeNode>  header_;
};



int main()
{
    RBTree<int> tree;


    tree.insert(1232);
    tree.insert(5);
    tree.insert(123);
    tree.insert(13);
    tree.insert(12133);
    tree.insert(59);
    tree.insert(123);
    tree.insert(1223);
    tree.insert(33);
    tree.insert(3);
    tree.insert(345);
    tree.insert(1257);
    tree.insert(1);
    tree.insert(1089);
    tree.insert(10489);
    tree.insert(89);
    tree.insert(10893);
    tree.insert(1012389);

    tree.show();
    return 0;
}






