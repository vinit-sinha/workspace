#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <set>
#include <cassert>
#include <fstream>
using namespace std;

struct Node{
   Node* next;
   Node* prev;
   int value;
   int key;
   Node(Node* p, Node* n, int k, int val):prev(p),next(n),key(k),value(val){};
   Node(int k, int val):prev(NULL),next(NULL),key(k),value(val){};
};

class Cache{
   
   protected: 
   map<int,Node*> mp; //map the key to the node in the linked list
   int cp;  //capacity
   Node* tail; // double linked list tail pointer
   Node* head; // double linked list head pointer
   virtual void set(int, int) = 0; //set function
   virtual int get(int) = 0; //get function

};
class LRUCache: public Cache {
public:
    LRUCache(int capacity) : Cache()
    {
        cp = capacity;
        tail = nullptr;
        head = nullptr;
        cacheSize = 0;
    }
    
    void set(int key, int value) {
        /*
        cout << "Setting (" << key << ", " << value << ") into [" ;
        Node* t = head;
        while( t != nullptr ) {
            cout << "(" << t->key << ", " << t->value << ") ";
            t = t->next;
        }
        cout << "]" << endl;
        */
        Node* node = quickGet(key);
        
        if( node == nullptr ) { //Store Miss
            node = new Node(key, value);
            node = saveInStore(node);
            saveInCache(node);
        }
        markAsMostRecentlyUsed(node);
    }
    
    int get(int key) {
        /*
        cout << "getting (" << key << ") from [" ;
        Node* t = head;
        while( t != nullptr ) {
            cout << "(" << t->key << ", " << t->value << ") ";
            t = t->next;
        }
        cout << "]" << endl;
        */
        Node* node = quickGet(key);
        if( node == nullptr ) { //Store miss
            return -1;
        } else {
            markAsMostRecentlyUsed(node);
            return node->value;
        }
    }
private:
    int cacheSize;
    
    Node* quickGet(int key) {
        auto iter = mp.find(key);
        return ( iter == mp.end() ? nullptr : iter->second );
    }
    
    //Knocks out least recently used, if ther is no room for node
    // effectlively increasing size by one, if knock out doesnt happen
    //  otherwise size remains unchanged
    void saveInCache(Node* node) {
        if( cacheSize == cp ) {
            removeLeastRecentlyUsed();
        }
        
        insertToHead(node);
    }
    
    Node* saveInStore(Node* node) {
        auto insResult = mp.emplace(node->key, node);
        auto iter = insResult.first;
        auto insertedNode = iter->second;
        
        if ( !insResult.second )  {
            insertedNode->value = node->value;
        }
        
        return insertedNode;
    }
    
    // Moves node from its current postion to head of the cache
    // effectlively leaving size of cache unchanged
    void markAsMostRecentlyUsed(Node* node) {
        Node* prevNode = node->prev;
        Node* nextNode = node->next;
        
        if( prevNode != nullptr && nextNode != nullptr ) { //Node in between head and tail
            prevNode->next = nextNode;
            nextNode->prev = prevNode;
            
            node->next = head;
            head->prev = node;
            node->prev = nullptr;
        } else if(prevNode == nullptr ) { // node already at head
            // Do Nothing
        } else if ( nextNode == nullptr ) { //Node at tail
            prevNode->next = nullptr;
            
            node->next = head;
            head->prev = node;
            node->prev = nullptr;
        }
        
        
    }
    
    void insertToHead(Node* node) {
        if( head == nullptr ) {
            head = node;
            tail = node;
        } else {
            node->next = head;
            node->prev = nullptr;
            head->prev = node;
            head = node;
        }
        
        cacheSize++;
    }
    
    void removeLeastRecentlyUsed() {
        if( tail == nullptr ) return;
        
        Node* node = tail;
        mp.erase(node->key);
        removeTail();
    }
    
    void removeTail() {
        Node* node = tail;
        // std::cout << "Removing tail (" << node->key << ", " << node->value << ")" << endl;
        tail = tail->prev;
        tail->next = nullptr;
        
        // Make sure the node no longer points to anybody in our store/cache
        node->next = nullptr;
        node->prev = nullptr;
        
        cacheSize--;
    }
};
int main(int argc, char** argv) {
   int n, capacity,i;
    if( argc != 2 ) return -1;
    ifstream in(argv[1]);
   in >> n >> capacity;
   LRUCache l(capacity);
   for(i=0;i<n;i++) {
      string command;
      in >> command;
      if(command == "get") {
         int key;
         in >> key;
         cout << l.get(key) << endl;
      } 
      else if(command == "set") {
         int key, value;
         in >> key >> value;
         l.set(key,value);
      }
   }
   return 0;
}

