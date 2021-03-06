#include "disk_ops.cpp"
using namespace std;

// Whenever we are fetching a AVLNODE, we are storing it in RAM as a BBST
// as a cache. This helps in improving performance
// With cache, insertion is much faster 
// Here we are making sure that data in main memory and disk are consistent
// Any changes to data present in RAM must be reflected on disk
// Similar to cache coherence problem

/**
 * 4 bytes insert address, next 4 bytes root address
 **/
int Initialize_AVLindex(char* name){
    int fd = open(name, O_WRONLY);
    int num = 2 * NODEPOINTER;
    write(fd, (void*)&num, NODEPOINTER);
    lseek(fd, NODEPOINTER, SEEK_SET);
    num = -1;
    write(fd, (void*)&num, NODEPOINTER);
    close(fd);
    return 1;
}

// Given a offset read 4 bytes from the given .avl file
int Read_4Bytes_Address(char* name, int offset){
    int fd = open(name, O_RDONLY);
    lseek(fd, offset, SEEK_SET);
    void* buffer = malloc(NODEPOINTER);
    read(fd, buffer, NODEPOINTER);
    int *p = (int*)buffer;
    int data = *p;
    free(buffer);
    close(fd);
    return data;
}

// Given .avl file, update the insert address to newAddress
int Update_Insert_Address(char* name, int newAddress){
    int fd = open(name, O_WRONLY);
    ssize_t bytes = write(fd, (void*)&newAddress, NODEPOINTER);
    close(fd);
    return 1;
}

// Given .avl file, update the Root address to newAddress
int Update_Root_Address(char* name, int newAddress){
    int fd = open(name, O_WRONLY);
    lseek(fd, 4, SEEK_SET);
    ssize_t bytes = write(fd, (void*)&newAddress, NODEPOINTER);
    close(fd);
    return 1;
}

// Given .avl file and offset, read 32 bytes
char* Read_32Bytes(char* name, int offset){
    int fd = open(name, O_RDONLY);
    lseek(fd, offset, SEEK_SET);
    char* res = (char*) malloc(STRING * sizeof(char));
    read(fd, (void*)res, STRING);
    close(fd);
    return res;
}

// Given .avl file, nodeNum which is address to insert and AVLNODE
// write AVLNODE starting from nodeNum address in the given .avl file
int Write_AVLNODE(char* name, int nodeNum, AVLNODE* node){
    int fd = open(name, O_WRONLY);
    int offset = nodeNum;
    ssize_t bytes;
    lseek(fd, offset, SEEK_SET);
    bytes = write(fd, (void*)&node->key, NODEPOINTER);
    offset += NODEPOINTER;

    lseek(fd, offset, SEEK_SET);
    bytes = write(fd, (void*)&node->parent, NODEPOINTER);
    offset += NODEPOINTER;

    lseek(fd, offset, SEEK_SET);
    bytes = write(fd, (void*)&node->left, NODEPOINTER);
    offset += NODEPOINTER;

    lseek(fd, offset, SEEK_SET);
    bytes = write(fd, (void*)&node->right, NODEPOINTER);
    offset += NODEPOINTER;

    lseek(fd, offset, SEEK_SET);
    bytes = write(fd, (void*)&node->height, NODEPOINTER);
    offset += NODEPOINTER;

    for(auto i: node->blocks){
        int val = i;
        lseek(fd, offset, SEEK_SET);
        bytes = write(fd, (void*)&val, NODEPOINTER);
        offset += NODEPOINTER;
    }
    for(int i = 0; i < (10 - node->blocks.size()); i++){
        lseek(fd, offset, SEEK_SET);
        int val = -1;
        bytes = write(fd, (void*)&val, NODEPOINTER);
        offset += NODEPOINTER;
    }
    close(fd);
    return 1;
}

// Read AVLNODE from address nodeNum in the given .avl file
AVLNODE* Read_AVLNODE(char* name, int nodeNum, int index){
    int offset = nodeNum;
    AVLNODE* node = new AVLNODE();
    node->key = Read_4Bytes_Address(name, offset);
    offset += NODEPOINTER;

    node->parent = Read_4Bytes_Address(name, offset);
    offset += NODEPOINTER;

    node->left = Read_4Bytes_Address(name, offset);
    offset += NODEPOINTER;

    node->right = Read_4Bytes_Address(name, offset);
    offset += NODEPOINTER;

    node->height = Read_4Bytes_Address(name, offset);
    offset += NODEPOINTER;

    set<int> blocks;

    for(int i = 0; i < 10; i++){
        int val = Read_4Bytes_Address(name, offset);
        if(val == -1) break;
        offset += NODEPOINTER;
        blocks.insert(val);
    }
    node->blocks = blocks;

    return node;
}

// Update a 4 byte data to newval in the given .avl file with offset addr
int Update_Inside_AVLNode(char* name, int addr, int newval){
    int fd = open(name, O_WRONLY);
    lseek(fd, addr, SEEK_SET);
    size_t bytes = write(fd, &newval, INTEGER);
    close(fd);
    return 1;
}

// Update AVLNODE header to header in the parameter in the file .avl with offset addr
int Update_AVLHeaders(char* name, int addr, vector<int> header){
    int fd = open(name, O_WRONLY);
    lseek(fd, addr, SEEK_SET);
    int offset = addr;
    for(int i = 0; i < (int)header.size(); i++){
        write(fd, &header[i], INTEGER);
        offset += INTEGER;
        lseek(fd, offset, SEEK_SET);
    }
    close(fd);
    return 1;
}

// Add a tuple's block address in the file .avl with offset addr
int Add_AVLBlock(char* name, int addr, int val){
    int fd = open(name, O_RDONLY);
    lseek(fd, addr, SEEK_SET);
    int offset = addr;
    for(int i = 0; i < 10; i++){
        int block;
        read(fd, &block, INTEGER);
        if(block == -1){
            close(fd);
            fd = open(name, O_WRONLY);
            lseek(fd, offset, SEEK_SET);
            write(fd, &val, INTEGER);
            close(fd);
            return 1;
        }
        offset += INTEGER;
        lseek(fd, offset, SEEK_SET);
    }
    close(fd);
    return 0;
}

// Replace a tuple's block address with value oldblock to newblock in the file .avl with offset addr
int Replace_AVLBlock(char* name, int addr, int oldblock, int newblock){
    int fd = open(name, O_RDONLY);
    lseek(fd, addr, SEEK_SET);
    int offset = addr;
    for(int i = 0; i < 10; i++){
        int block;
        read(fd, &block, INTEGER);
        if(block == oldblock){
            close(fd);
            fd = open(name, O_WRONLY);
            lseek(fd, offset, SEEK_SET);
            write(fd, &newblock, INTEGER);
            close(fd);
            return 1;
        }
        offset += INTEGER;
        lseek(fd, offset, SEEK_SET);
    }
    close(fd);
    return 0;
}

// Delete a tuple's block address with value block in the given .avl file with offset addr
int Delete_AVLBlock(char* name, int addr, int block){
    int fd = open(name, O_RDONLY);
    lseek(fd, addr, SEEK_SET);
    int offset = addr;
    set<int> blocks;
    for(int i = 0; i < 10; i++){
        int b;
        read(fd, &b, INTEGER);
        if(b == -1){
            break;
        }
        blocks.insert(b);
        offset += INTEGER;
        lseek(fd, offset, SEEK_SET);
    }
    close(fd);
    blocks.erase(block);
    fd = open(name, O_WRONLY);
    offset = addr;
    lseek(fd, offset, SEEK_SET);
    for(int i: blocks){
        write(fd, &i, INTEGER);
        offset += INTEGER;
        lseek(fd, offset, SEEK_SET);
    }
    int b = -1;
    for(int i = 0; i < (10 - (int)blocks.size()); i++){
        write(fd, &b, INTEGER);
        offset += INTEGER;
        lseek(fd, offset, SEEK_SET);
    }
    close(fd);
    return 0;
}

// Fetch a AVLNODE
// If present in the main memory, fetch it from there
// If absent in the main memory, fetch from disk
AVLNODE* FetchAVLNodeCache(char* name, int address, int indexNum){
    // cout << "start" << endl;
    AVLNODE* node;
    if(avlnodes.find({indexNum, address}) != avlnodes.end())
        node = avlnodes[{indexNum, address}];
    else{
        node = Read_AVLNODE(name, address, 0);
        if((int)avlnodes.size() >= AVLCACHESIZE){
            AVLNODE* temp = avlnodes.begin()->second;
            avlnodes.erase(avlnodes.begin());
            delete temp;
        }
        avlnodes[{indexNum, address}] = node;
    }
    // cout << "stop" << endl;
    return node;
}

void UpdateAVLNodeCache(AVLNODE* node, int address){
}

// Given a .avl file, disk address of some AVLNODE,
// find the height and return
int getHeight(char* name, int nodeNum, int indexNum){
    if(nodeNum == -1) return -1;
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    int height = node->height;
    return height;
}

// Same as Rotate right operation of AVL in memory implementation
// name = .avl file
// nodeNum = disk address of that avl node in the given avl file
// indexNum = position of attribute in which index is created
int RotateRight(char* name, int nodeNum, int indexNum){
    AVLNODE* node =FetchAVLNodeCache(name, nodeNum, indexNum);
    AVLNODE* left = FetchAVLNodeCache(name, node->left, indexNum);
    int leftAddr = node->left;
    left->parent = node->parent;
    node->parent = node->left;
    node->left = left->right;
    left->right = nodeNum;
    node->height = max(getHeight(name, node->right, indexNum) + 1, getHeight(name, node->left, indexNum) + 1);
    left->height = max(node->height + 1, getHeight(name, left->left, indexNum) + 1);
    Update_AVLHeaders(name, nodeNum + INTEGER, {node->parent, node->left, node->right, node->height});
    Update_AVLHeaders(name, leftAddr + INTEGER, {left->parent, left->left, left->right, left->height});
    int nodeAddress = node->parent;
    avlnodes[{indexNum, nodeNum}] = node;
    avlnodes[{indexNum, leftAddr}] = left;
    return nodeAddress;
}

// Same as Rotate left operation of AVL in memory implementation
// name = .avl file
// nodeNum = disk address of that avl node in the given avl file
// indexNum = position of attribute in which index is created
int RotateLeft(char* name, int nodeNum, int indexNum){
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    AVLNODE* right = FetchAVLNodeCache(name, node->right, indexNum);
    int rightAddr = node->right;
    right->parent = node->parent;
    node->parent = node->right;
    node->right = right->left;
    right->left = nodeNum;
    node->height = max(getHeight(name, node->right, indexNum) + 1, getHeight(name, node->left, indexNum) + 1);
    right->height = max(getHeight(name, right->right, indexNum) + 1, node->height + 1);
    Update_AVLHeaders(name, nodeNum + INTEGER, {node->parent, node->left, node->right, node->height});
    Update_AVLHeaders(name, rightAddr + INTEGER, {right->parent, right->left, right->right, right->height});
    int nodeAddress = node->parent;
    avlnodes[{indexNum, nodeNum}] = node;
    avlnodes[{indexNum, rightAddr}] = right;
    return nodeAddress;
}

// Same a balancing in avl tree
// name = .avl file
// nodeNum = disk address of that avl node in the given avl file
// indexNum = position of attribute in which index is created
int BalanceTree(char* name, int nodeNum, int indexNum){
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    int left = getHeight(name, node->left, indexNum) + 1,
    right = getHeight(name, node->right, indexNum) + 1;
    int resAddr = nodeNum;
    if(left - right > 1){
        // left
        AVLNODE* leftNode = FetchAVLNodeCache(name, node->left, indexNum);
        left = getHeight(name, leftNode->left, indexNum) + 1;
        right = getHeight(name, leftNode->right, indexNum) + 1;
        if(left >= right){
            // left left case
            resAddr = RotateRight(name, nodeNum, indexNum);
        }
        else{
            node->left = RotateLeft(name, node->left, indexNum);
            Update_Inside_AVLNode(name, nodeNum + 2 * INTEGER, node->left);
            avlnodes[{indexNum, nodeNum}] = node;
            resAddr = RotateRight(name, nodeNum, indexNum);
        }
    }
    else if(right - left > 1){
        // right
        AVLNODE* rightNode = Read_AVLNODE(name, node->right, 0);
        left = getHeight(name, rightNode->left, indexNum) + 1;
        right = getHeight(name, rightNode->right, indexNum) + 1;
        if(left >= right){
            node->right = RotateRight(name, node->right, indexNum);
            Update_Inside_AVLNode(name, nodeNum + 3 * INTEGER, node->right);
            avlnodes[{indexNum, nodeNum}] = node;
            resAddr = RotateLeft(name, nodeNum, indexNum);
        }
        else{
            resAddr = RotateLeft(name, nodeNum, indexNum);
        }
    }
    return resAddr;
}

// Same as Insertion in avl tree
// name = .avl file
// nodeNum = disk address of that avl node in the given avl file
// parent = parent of node present at nodeNum
// value = key
// block_point address of tuple
// indexNum = position of attribute in which index is created
int Insert_INT_AVLindex(char* name, int nodeNum, int parent, int value, int block_pointer, int indexNum){
    // cout << "nodeNum: " << nodeNum << endl;
    if(nodeNum == -1){
        set<int> blocks;
        blocks.insert(block_pointer);
        AVLNODE* node = GetAVLNODE(value, parent, -1, -1, blocks);
        int insertNum = Read_4Bytes_Address(name, 0);
        Write_AVLNODE(name, insertNum, node);
        Update_Insert_Address(name, insertNum + AVLNODESIZE);
        return insertNum;
    }
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    if(node->key == value){
        cout << "present" << endl;
        // node->size++;
        Add_AVLBlock(name, nodeNum + 5 * INTEGER, block_pointer);
        node->blocks.insert(block_pointer);
        avlnodes[{indexNum, nodeNum}] = node;
        cout << "size: " << (int)avlnodes[{indexNum, nodeNum}]->blocks.size() << endl;
    }
    else if(value < node->key){
        int left = Insert_INT_AVLindex(name, node->left, nodeNum, value, block_pointer, indexNum);
        node->left = left;
        avlnodes[{indexNum, nodeNum}] = node;
        AVLNODE* leftNode = FetchAVLNodeCache(name, left, indexNum);
        // node->height = max(node->height, leftNode->height + 1);
        node->height = max(getHeight(name, node->left, indexNum) + 1, getHeight(name, node->right, indexNum) + 1);
        avlnodes[{indexNum, nodeNum}] = node;
        Update_Inside_AVLNode(name, nodeNum + 2 * INTEGER, node->left);
        Update_Inside_AVLNode(name, nodeNum + 4 * INTEGER, node->height);
        avlnodes[{indexNum, nodeNum}] = node;
        // delete leftNode;
    }
    else{
        int right = Insert_INT_AVLindex(name, node->right, nodeNum, value, block_pointer, indexNum);
        node->right = right;
        avlnodes[{indexNum, nodeNum}] = node;
        AVLNODE* rightNode = FetchAVLNodeCache(name, right, indexNum);
        // node->height = max(node->height, rightNode->height + 1);
        node->height = max(getHeight(name, node->left, indexNum) + 1, getHeight(name, node->right, indexNum) + 1);
        avlnodes[{indexNum, nodeNum}] = node;
        Update_Inside_AVLNode(name, nodeNum + 3 * INTEGER, node->right);
        Update_Inside_AVLNode(name, nodeNum + 4 * INTEGER, node->height);
        avlnodes[{indexNum, nodeNum}] = node;
        // delete rightNode;
    }
    // Write_AVLNODE(name, nodeNum, node);
    nodeNum = BalanceTree(name, nodeNum, indexNum);
    // delete node;
    return nodeNum;
}

// Same as search in avl tree
// paramters are same as above
int Search_INT_AVLindex(char* name, int nodeNum, int parent, int value, int indexNum){
    // cout << "nodeNum: " << nodeNum << endl;
    if(nodeNum == -1) return -1;
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    // cout << "node->intkey: " << node->intkey << endl;
    int res = -1;
    if(node->key == value){
        res = nodeNum;
    }
    else if(value < node->key){
        res = Search_INT_AVLindex(name, node->left, nodeNum, value, indexNum);
    }
    else{
        res = Search_INT_AVLindex(name, node->right, nodeNum, value, indexNum);
    }
    // delete node;
    return res;
}

// name = .avl file
// nodeNum = disk address of that avl node in the given avl file
// indexNum = position of attribute in which index is created
// find maximum value in the tree rooted at nodeNum
int GetMaximum_INT_AVLindex(char* name, int nodeNum, int indexNum){
    if(nodeNum == -1) return -1;
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    int right = node->right;
    if(right == -1) return nodeNum;
    else return GetMaximum_INT_AVLindex(name, right, indexNum);
}

// name = .avl file
// nodeNum = disk address of that avl node in the given avl file
// indexNum = position of attribute in which index is created
// find minimum value in the tree rooted at nodeNum
int GetMinimum_INT_AVLindex(char* name, int nodeNum, int indexNum){
    if(nodeNum == -1) return -1;
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    int left = node->left;
    // delete node;
    if(left == -1) return nodeNum;
    else return GetMinimum_INT_AVLindex(name, left, indexNum);
}

// delete node from avl tree present at the given .avl file
// parameters same as stated above
int Delete_INT_AVLindex(char* name, int nodeNum, int parent, int value, int indexNum){
    AVLNODE* node = FetchAVLNodeCache(name, nodeNum, indexNum);
    if(node->key == value){
        int inpre = GetMaximum_INT_AVLindex(name, node->left, indexNum);
        if(inpre == -1){
            int insucc = GetMinimum_INT_AVLindex(name, node->right, indexNum);
            if(insucc == -1){
                // this is a leaf
                return -1;
                // if(parent != -1){
                //     AVLNODE* par = Read_AVLNODE(name, parent, 0);
                //     if(par->left == nodeNum) par->left = -1;
                //     else par->right = -1;
                //     Write_AVLNODE(name, parent, par);
                    
                // }
            }
            else{
                AVLNODE* insuccnode = FetchAVLNodeCache(name, insucc, indexNum);
                // avlnodes.erase({indexNum,insucc});
                insuccnode->right = Delete_INT_AVLindex(name, node->right, nodeNum, insuccnode->key, indexNum);
                insuccnode->left = node->left;
                insuccnode->parent = node->parent;
                insuccnode->height = max(getHeight(name, insuccnode->left, indexNum) + 1, 
                                getHeight(name, insuccnode->right, indexNum) + 1);
                Write_AVLNODE(name, nodeNum, insuccnode);
                avlnodes[{indexNum,nodeNum}] = insuccnode;
            }
        }
        else{
            AVLNODE* inprenode =FetchAVLNodeCache(name, inpre, indexNum);
            // avlnodes.erase({indexNum,inpre});
            inprenode->left = Delete_INT_AVLindex(name, node->left, nodeNum, inprenode->key, indexNum);
            inprenode->right = node->right;
            inprenode->parent = node->parent;
            inprenode->height = max(getHeight(name, inprenode->left, indexNum) + 1, 
                                getHeight(name, inprenode->right, indexNum) + 1);
            Write_AVLNODE(name, nodeNum, inprenode);
            avlnodes[{indexNum,nodeNum}] = inprenode;
        }
    }
    else if(value < node->key){
        node->left = Delete_INT_AVLindex(name, node->left, nodeNum, value, indexNum);
        node->height = max(getHeight(name, node->left, indexNum) + 1, 
                                getHeight(name, node->right, indexNum) + 1);
        Write_AVLNODE(name, nodeNum, node);
        avlnodes[{indexNum,nodeNum}] = node;
    }
    else{
        node->right = Delete_INT_AVLindex(name, node->right, nodeNum, value, indexNum);
        node->height = max(getHeight(name, node->left, indexNum) + 1, 
                                getHeight(name, node->right, indexNum) + 1);
        Write_AVLNODE(name, nodeNum, node);
        avlnodes[{indexNum,nodeNum}] = node;
    }
//     delete node;
    node = FetchAVLNodeCache(name, nodeNum, indexNum);
    Write_AVLNODE(name, nodeNum, node);
    nodeNum = BalanceTree(name, nodeNum, indexNum);
    // delete node;
    return nodeNum;
}

// Delete a block from the given avl node
// If that is the only block, then delete the avl node completely
// Else just remove the block from set and leave
int Delete_AVLBlock(char* name, int root, int block, int value, int indexNum){
    int address = Search_INT_AVLindex(name, root, -1, value, indexNum);
    if(address == -1){
        return 0;
    }
    AVLNODE* node = FetchAVLNodeCache(name, address, indexNum);
    if((int)node->blocks.size() == 1){
        // cout << "here in if " << (int)node->blocks.size() << " " << node->key << endl;
        root = Delete_INT_AVLindex(name, root, -1, value, indexNum);
        Update_Root_Address(name, root);
        if(avlnodes.find({indexNum, address}) != avlnodes.end())
            avlnodes.erase({indexNum, address});
        return 1;
    }
    // cout << "here not in if " << (int)node->blocks.size() << " " << node->key << endl;
    Delete_AVLBlock(name, address + 5 * INTEGER, block);
    node->blocks.erase(block);
    avlnodes[{indexNum, address}] = node;
    return 1;
}

void print_AVLnode(AVLNODE* node){
    cout << "intkey: " << node->key << endl;
    // cout << "stringkey: " << node->stringkey << endl;
    // cout << "parent: " << node->parent << endl;
    // cout << "left: " << node->left << endl;
    // cout << "right: " << node->right << endl;
    // cout << "size: " << node->size << endl;
    // cout << "index: " << node->index << endl;
    // cout << "height: " << node->height << endl;
    // cout << "blocks" << endl;
    for(auto i: node->blocks){
        cout << i << " ";
    }
    cout << endl;
}

void inorder(char* name, int nodeNum){
    if(nodeNum == -1) return;
    AVLNODE* node = Read_AVLNODE(name, nodeNum, 0);
    inorder(name, node->left);
    // cout << node->intkey << endl;
    cout << "Address: " << nodeNum << endl;
    print_AVLnode(node);
    inorder(name, node->right);
}

void test1(char* name, int indexNum){
    // set<int> a;
    // a.insert(1);
    // a.insert(2);
    // // char* name = (char*) malloc(STRING * sizeof(char));
    // // strcpy(name, "Student.avl");
    // // test1(name);
    // AVLNODE* node = GetAVLNODE(1, name, -1, -1, -1, 2, 0, a);
    // // cout << node->stringkey << endl;
    // Initialize_AVLindex(name);
    // int nodeNum = Read_4Bytes_Address(name, 0);
    // cout << nodeNum << endl;
    // // Write_AVLNODE(name, nodeNum, node);
    // cout << "here" << endl;
    // // free(node);
    // node = Read_AVLNODE(name, nodeNum, 0);
    // cout << "here" << endl;
    // // cout << node->index << endl;
    // print_AVLnode(node);
    // int root = Read_4Bytes_Address(name, 4);
    // inorder(name, root);
    while(1){
        // cout << "meta" << endl;
        // cout << Read_4Bytes_Address(name, 0) << " " << Read_4Bytes_Address(name, 4) << endl;
        // cout << "reading root" << endl;
        int root = Read_4Bytes_Address(name, 4);
        // cout << "root: " << root << endl;
        cout << "Enter value block_pointer: " << endl;
        int value, block_pointer;
        cin >> value >> block_pointer;
        cout << "here" << endl;
        root = Insert_INT_AVLindex(name, root, -1, value, block_pointer, indexNum);
        cout << "root: " << root << endl;
        Update_Root_Address(name, root);
        cout << "inorder" << endl;
        inorder(name, root);
        // break;
    }
}

// int main(){
    // // set<int> a;
    // // a.insert(1);
    // // a.insert(2);
    // char* name = (char*) malloc(STRING * sizeof(char));
    // strcpy(name, "Student.avl");
    // test1(name);
    // // AVLNODE* node = GetAVLNODE(1, name, -1, -1, -1, 2, 0, a);
    // // // cout << node->stringkey << endl;
    // // // Initialize_AVLindex(name);
    // // int nodeNum = Read_4Bytes_Address(name, 0);
    // // cout << nodeNum << endl;
    // // Write_AVLNODE(name, nodeNum, node);
    // // cout << "here" << endl;
    // // // free(node);
    // // node = Read_AVLNODE(name, nodeNum, 0);
    // // cout << "here" << endl;
    // // cout << node->index << endl;
// }