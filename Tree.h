#ifndef TREE_H
#define TREE_H

#include "config.h"
#include "Hyperrectangle.h"

using namespace std;

template<typename Container>
class Tree
{
private:
    typedef pair<Hyperrectangle, void *> entry;
    typedef vector<entry> aentry;
    class Node
    {
        friend class Tree;
    private:
        Node * parent;
        size_t level;
        aentry entries;
    public:
        Node(Node *parent, size_t level)
        {
            this->parent = parent;
            this->level = level;
        }
    };
    
    Node * head;
    size_t nDimensions;

public:
    Tree(size_t nDimensions)
    {
        this->head = new Node(nullptr,0);
        this->nDimensions = nDimensions;
    }

    bool insert(Hyperrectangle BoundingBox, Container &data)
    {
      Node * s;
      Container * d = new Container;
      *d = data;
      this->chooseLeaf(BoundingBox, *d, s);

      if(s->entries.size() < CHILDS){
        s->entries.push_back(make_pair(BoundingBox,static_cast<void *>(d)));
        //cout << s->entries[s->entries.size()-1] << endl;
        return true;
      }

      Node a(s->parent, s->level);
      Node *tmp = &a;
      entry n = make_pair(BoundingBox, static_cast<void *>(d));
      this->split(s,&a,n);
      cout << s->entries.size() << "   " << a.entries.size() << endl;
      this->AdjustTree(&s, &tmp);

      return true;
    }

    size_t IdxNode(Node * &parent, Node * &child)
    {
        for (size_t n_idx;n_idx<parent->entries.size();n_idx++)
        {
            if (parent->entries[n_idx].second == child)
            {
                return n_idx;
            }
        }
    }

    bool chooseLeaf(Hyperrectangle BoundingBox, Container data, Node* &s)
    {
        s = this->head;
        double min;
        size_t use;
        while(s->level!=0){
            min = INF;
            use = 0;
            for(int i=0;i<s->entries.size();i++)
                if(s->entries[i].first.contain(BoundingBox) < min)
                    use = i;
            s = static_cast<Node *>(s->entries[use].second);
        }
        return true;
    }

    Hyperrectangle Adjust(Node * & s)
    {
        points mbr_points=s->entries[0].first.vertices;

        mbr_points[0].first;

        for (std::vector< pair<Hyperrectangle, void *> >::iterator i = ++(s->entries.begin()); i != s->entries.end(); ++i)
        {
            for (size_t j=0;j<nDimensions;j++ )
            {
                if((mbr_points[j].first)>((*i).first.vertices[j].first))
                {
                    mbr_points[j].first=(*i).first.vertices[j].first;
                }
                if((mbr_points[j].second)<((*i).first.vertices[j].second))
                {
                    mbr_points[j].second=(*i).first.vertices[j].second;
                }
            }

        }
        return Hyperrectangle(mbr_points);
    }

    void AdjustTree(Node **  l,Node **  splitted)
    {
        Node ** n= &(*l);
        if ((*n) == this->head)
        {
            return;
        }

        Node ** parent = &((*n)->parent);
        size_t n_idx=IdxNode(*parent,*n);
        (*parent)->entries[n_idx].first=Adjust(*n);
        if (splitted != NULL)
        {
            if ((*splitted)->entries.size()< CHILDS-1)
            {
                typename Tree::entry new_entry;
                Node ** sp_parent =&((*splitted)->parent);
                new_entry.second=(void*)(*splitted);
                new_entry.first=(*sp_parent)->entries[IdxNode(*sp_parent,*splitted)].first;
                (*parent)->entries.push_back(new_entry);
                return AdjustTree(parent,NULL);

            }
            else
            {
               //return AdjustTree(parent,Parent);
            }
        }
        else
        {
            return AdjustTree(parent, NULL);
        }


    }

    bool Search(Hyperrectangle BoundingBox, vector<Container> &stack)
    {

        Node **s;
        s=&this->head;
        return find(BoundingBox,s,stack);
    }

    bool find(Hyperrectangle BoundingBox, Node** &s, vector<Container> &stack)
    {

        if ((*s)->level == 0)
        {
            for(int i=0;i<(*s)->entries.size();i++)
            {
                if((*s)->entries[i].first.overlap(BoundingBox))
                {
                    stack.push_back(*static_cast<Container *>((*s)->entries[i].second));
                }
            }
            return true;
        }

        for (int i=0;i<(*s)->entries.size();i++)
        {
            if((*s)->entries[i].first.overlap(BoundingBox))
            {
                s=(Node**) (& (*s)->entries[i].second);
                find(BoundingBox,s,stack);
            }
        }

        return true;

    }

    bool split(Node *l, Node *ll, entry g)
    {
        aentry group;

        for(int i=0;i<l->entries.size();i++)
        {
            group.push_back(l->entries[i]);
        }
        group.push_back(g);

        l->entries.clear();

        int e1, e2;
        pickSeeds(group, e1, e2);

        l->entries.push_back(group[e1]);
        ll->entries.push_back(group[e2]);
        Hyperrectangle h1(group[e1].first.getVertices()), h2(group[e1].first.getVertices());

        group.erase(group.begin() + e1);
        group.erase(group.begin() + e2);

        int e, ng;
        while(group.size())
        {
            pickNext(group, &h1, &h2, e, ng);
            if(ng < 1)
            {
                l->entries.push_back(group[e]);
                h1.contain(group[e].first);
            }else{
                ll->entries.push_back(group[e]);
                h2.contain(group[e].first);
            }
            group.erase(group.begin() + e);
        }
    }

    void pickSeeds(aentry block, int &e1, int &e2 )
    {
        double d = 0;
        double tmp;
        cout << block.size() << endl;
        for(int i=0;i<block.size();i++)
        {
            for(int j=i+1;j<block.size();j++)
            {
                tmp = abs(block[i].first.contain(block[j].first) - block[i].first.area() - block[j].first.area()) ;
                if(d < (tmp))
                {
                    e1 = i;
                    e2 = j;
                    d = tmp;
                }
            }
        }
    }

    void pickNext(aentry noGroup, Hyperrectangle * g1, Hyperrectangle * g2, int &e, int&ng)
    {
        double m = -1;
        double d1;
        double d2;
        ng = 0;

        for(int i=0;i<noGroup.size();i++)
        {
            d1 = noGroup[i].first.contain(*g1);
            d2 = noGroup[i].first.contain(*g2);

            if(m < abs(d1 - d2))
            {
                m = abs(d1 - d2);
                e = i;
                if(d1 > d2)
                {
                    ng = 0;
                }else{
                    ng = 1;
                }
            }
        }
    }

     Node* FindLeaf(entry a,Node * n )
     {
        if (n->level == 0 )
        {
            size_t idx_n= IdxNode(n->parent,n);
            if(n->parent->entries[idx_n].first.contain(a.first))
                return n;
            return NULL;
        }
        else
        {
            for(vector<entry>::iterator i = n->entries.begin();i!=n->entries.end();i++)
            {

                if ((*i).first.overlap(a.first))
                {
                    Node * l= FindLeaf(a,(*i).second);
                    if (l != NULL)
                    {
                            return l;
                    }
                }
            }

        }

     }

     void CondenseTree(Node * &n, vector<Node*>& qs)
     {

         if (n!= this->head)
         {
             Node* parent= n->parent;
             size_t idx_p=IdxNode(n->parent,n);
             entry* e_parent = &(n->parent->entries[idx_p].first);
             if (n->entries.size()< 2 /* 2 es solo un número, hay que especificar un m y un M en el arbol*/)
             {
                 delete e_parent;
                 qs.push_back(n);
             }
             else
             {
                 n->parent->entries[idx_p].first= Adjust(n);
             }
             CondenseTree(parent,qs);
         }
         else if ((n == this->head) && (qs.size!=0))
         {
             for (typename vector<Node*>::iterator i= qs.begin();i!=qs.end();i++)
             {
                    for(vector<entry>::iterator j=(*i).entries.begin(); j!=(*i).entries.end();j++ )
                    {
                        if ((*i).level==0)
                        {
                            insert((*j).first,(Container)((*j).second));
                        }
                    }
             }
         }


     }

     bool Delete(entry &e)
     {
         Node **p = &(this->head);
         return Del(e,p);

     }

     bool Del(entry &e, Node** p)
     {

         Node * l = FindLeaf(e,*p);
         delete &e;
         vector<Node*> qs;
         CondenseTree(l,qs);
         if(this->head->entries.size()==1)
         {
             head=&(this->head->entries[0].second);
         }
     }
};




#endif // TREE_H
