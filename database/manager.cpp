#include "avl_ops.cpp"

vector<int> index_attr; // assuming it is sorted
vector<char*> names;

void InitializeIndexes(){
    for(int i = 0; i < (int)index_attr.size(); i++){
        cout << names[i] << endl;
        create_file(names[i]);
        Initialize_AVLindex(names[i]);
    }
}

void Initialize_DB(char* name, char* meta){
    if((int)index_attr.size() > 0) return;
    index_attr = Read_Index(meta);
    for(int i = 0; i < (int)index_attr.size(); i++){
        string curr = name;
        curr += to_string(index_attr[i]);
        curr += ".avl";
        names.push_back((char*) malloc(STRING * sizeof(char)));
        strcpy(names[i], curr.c_str());
    }
    // for(int i = 0; i < (int)index_attr.size(); i++){
    //     cout << names[i] << endl;
    //     create_file(names[i]);
    //     Initialize_AVLindex(names[i]);
    // }
}

int Update_Index(char* name, char* meta, vector<AttributeNode*> tuple, int blockaddr){
    if((int)names.size() == 0){
        Initialize_DB(name, meta);
    }
    for(int i = 0; i < (int)names.size(); i++){
        int key;
        if(tuple[index_attr[i]]->index == 0){
            key = tuple[index_attr[i]]->num;
        }
        int root = Read_4Bytes_Address(names[i], 4);
        root = Insert_INT_AVLindex(names[i], root, -1, key, blockaddr);
        Update_Root_Address(names[i], root);
    }
    return 1;
}

void PrintTuple(vector<AttributeNode*> tuple){
    for(AttributeNode* node: tuple){
        if(node->index == 0) cout << node->num << " ";
        else if(node->index == 1) cout << node->b << " ";
        else cout << node->str << " ";
    }
    cout << endl;
}

vector<int> Search_Index(int i, int val){
    vector<int> arr;
    int root = Read_4Bytes_Address(names[i], 4);
    cout << "here" << endl;
    cout << names[i] << " " << val << endl;
    int avlNodeLoc = Search_INT_AVLindex(names[i], root, -1, val);
    cout << "here" << endl;
    cout << "avlNodeLoc: " << avlNodeLoc << endl;
    if(avlNodeLoc != -1){
        AVLNODE* node = Read_AVLNODE(names[i], avlNodeLoc, 0);
        for(auto i: node->blocks){
            arr.push_back(i);
        }
    }
    return arr;
}

int BinarySearch(int val){
    int l = 0, u = (int)index_attr.size() - 1;
    while(l <= u){
        int mid = (l + u) / 2;
        if(index_attr[mid] == val) return mid;
        else if(val < index_attr[mid]) u = mid - 1;
        else l = mid + 1;
    }
    return -1;
}

// vector<int> Search_EntireDB(char* dbname, )

void FreeTuple(vector<AttributeNode*> tuple){

}

vector<int> GetPositions(vector<pair<string,string>> schema, vector<pair<string,string>> arr){
    vector<int> positions((int)arr.size());
    for(int i = 0; i < (int)arr.size(); i++){
        for(int j = 0; j < (int)schema.size(); j++){
            if(schema[j].first == arr[i].first){
                positions[i] = j;
                break;
            }
        }
    }
    return positions;
}

vector<int> Search_Database(char* name, char* dbname, char* meta, vector<pair<string,string>> schema, vector<pair<string,string>> arr){
    vector<int> positions = GetPositions(schema, arr);
    Initialize_DB(name, meta);
    cout << "here" << endl;
    for(int i: positions){
        cout << i << " ";
    }
    cout << endl;
    // cout << 
    for(int i: index_attr){
        cout << i << " ";
    }
    cout << endl;
    for(char* i: names){
        cout << i << " ";
    }
    cout << endl;
    for(int i = 0; i < (int)arr.size(); i++){
        int bs = BinarySearch(positions[i]);
        if(bs != -1){
            cout << "found index" << endl;
            vector<int> blocks = Search_Index(bs, stoi(arr[i].second));
            cout << "found index" << endl;
            cout << "blocks: " << endl;
            for(int i: blocks) cout << i << " ";
            cout << endl;
            vector<int> res;
            for(int i = 0; i < (int)blocks.size(); i++){
                vector<AttributeNode*> tuple = Read_Data_File(dbname, schema, blocks[i] / getTupleSize(schema), 
                                                getTupleSize(schema));
                PrintTuple(tuple);
                int index = 0;
                bool ok = true;
                for(int k = 0; k < (int)tuple.size() && index < (int)arr.size(); k++){
                    if(k == positions[index]){
                        if(tuple[k]->index == 0){
                            cout << "reached" << endl;
                            cout << tuple[k]->num << " " << stoi(arr[index].second) << endl;
                            if(tuple[k]->num == stoi(arr[index].second)){
                                index++;
                                cout << "reached" << endl;
                            }
                            else{
                                ok = false;
                                break;
                            }
                        }
                        else if(tuple[k]->index == 1){
                            if(tuple[k]->b == stoi(arr[index].second)){
                                index++;
                            }
                            else{
                                ok = false;
                                break;
                            }
                        }
                        else{
                            if(tuple[k]->str == arr[index].second){
                                index++;
                            }
                            else{
                                ok = false;
                                break;
                            }
                        }
                    }
                }
                if(ok){
                    res.push_back(blocks[i]);
                }
                FreeTuple(tuple);
            }
            return res;
        }
    }
    return {};
}

int Delete_In_Database(char* name, char* dbname, char* meta, vector<pair<string,string>> schema, vector<pair<string,string>> arr){
    vector<int> blocks = Search_Database(name, dbname, meta, schema, arr);
    for(int i: blocks){
        int tupleNum = i / getTupleSize(schema), 
        tupleSize = getTupleSize(schema);
        Delete_Data_Tuple(dbname, meta, schema, tupleNum, tupleSize);
        
        vector<AttributeNode*> tuple = Read_Data_File(dbname, schema, tupleNum, tupleSize);
        for(int j = 0; j < (int)tuple.size(); j++){
            int bs = BinarySearch(j);
            if(bs == -1) continue;

        }
    }
}

int main(){
    /**
     * schema creation syntax
     * 1 create schema database_name(attr type, attr type, ......)
     * 
     * insert syntax
     * 2 database_name attr1value attr2value attr3value ........
     * 
     * print all tuples syntax
     * 3 database_name
     * 
     * print meta and schema file contents syntax
     * 4 database_name
     * 
     * Delete syntax
     * 5 database_name index_of_the_tuple (0 based index)
     * 
     * Put Indexing details
     * 6 database_name attr0 attr1 ..... 
     * 
     * Search database
     * 7 database_name attr1 value attr2 value ....
     * 
     * Delete from database
     * 8 database_name attr1 value attr2 value ....
     **/
    while(1){
        cout << "enter query: ";
        string query;
        getline(cin, query);
        if(query[0] == '1'){
            int sp = query.find(' ');
            query = query.substr(sp + 1, query.size() - sp);
            vector<pair<string,string>> schema = parse_schema_DDL(query);
            string dbname = getDatabaseName(query);
            char* name = (char*) malloc(STRING * sizeof(char));
            strcpy(name, (dbname + ".schema").c_str());
            create_file(name);
            cout << "created schema file" << endl;
            Write_In_Schema_File(name, schema);
            int tupleSize = getTupleSize(schema);
            cout << "wrote schema in schema file" << endl;
            strcpy(name, (dbname + ".meta").c_str());
            create_file(name);
            cout << "created metadata file" << endl;
            Initialize_Meta_File(name, tupleSize, dbname);
            cout << "initialized metadata file" << endl;
            tupleSize = Read_Tuple_Size(name);
            cout << "tupleSize: " << tupleSize << endl;
            strcpy(name, (dbname + ".db").c_str());
            create_file(name);
            cout << "created database file" << endl;
            strcpy(name, (dbname + ".avl").c_str());
            create_file(name);


            Initialize_AVLindex(name);


            cout << "created avl index file" << endl;
            strcpy(name, (dbname + ".btree").c_str());
            create_file(name);
            cout << "created B-Tree index file" << endl;
            free(name);
        }
        else if(query[0] == '2'){
            int sp = query.find(' ') + 1;
            string dbname = query.substr(sp, query.find(' ', sp) - sp);
            char* name = (char*) malloc(STRING * sizeof(char));
            char* meta = (char*) malloc(STRING * sizeof(char));
            strcpy(name, (dbname + ".schema").c_str());
            vector<AttributeNode*> tuple = getTuple(Read_Schema_File(name), query.substr(query.find(' ', sp) + 1, query.size() - query.find(' ', sp)));
            int tupleSize = getTupleSize(Read_Schema_File(name));
            strcpy(name, (dbname + ".db").c_str());
            strcpy(meta, (dbname + ".meta").c_str());
            int blockAddr = Write_In_Data_File(name, meta, tuple, tupleSize);
            strcpy(name, dbname.c_str());
            Update_Index(name, meta, tuple, blockAddr);
            free(name);
            free(meta);
        }
        else if(query[0] == '3'){
            char* name = (char*) malloc(STRING * sizeof(char));
            char* meta = (char*) malloc(STRING * sizeof(char));
            string dbname = query.substr(query.find(' ') + 1, query.size() - query.find(' '));
            strcpy(meta, (dbname + ".meta").c_str());
            strcpy(name, (dbname + ".db").c_str());
            int size = Read_Num_Tuples(meta);
            strcpy(meta, (dbname + ".schema").c_str());
            vector<pair<string,string>> schema = Read_Schema_File(meta);
            cout << "Tuple: " << endl;
            for(int i = 0; i < size; i++){
                vector<AttributeNode*> tuple = Read_Data_File(name, schema, i, getTupleSize(schema));
                PrintTuple(tuple);
                FreeTuple(tuple);
            }
            free(name);
            free(meta);
        }
        else if(query[0] == '4'){
            string dbname = query.substr(query.find(' ') + 1, query.size() - query.find(' '));
            char* meta = (char*) malloc(STRING * sizeof(char));
            strcpy(meta, (dbname + ".meta").c_str());
            cout << "Last tuple address: " << Read_Last_Tuple_Address(meta) << endl;
            cout << "Database size: " << Read_Database_Size(meta) << endl;
            cout << "Number of Tuples: " << Read_Num_Tuples(meta) << endl;
            cout << "Tuple Size: " << Read_Tuple_Size(meta) << endl;
            cout << "Database name: " << Read_DatabaseName(meta) << endl;
            strcpy(meta, (dbname + ".schema").c_str());
            cout << "Schema: " << endl;
            vector<pair<string,string>> schema = Read_Schema_File(meta);
            for(pair<string,string> p: schema){
                cout << p.first << " " << p.second << endl;
            }
        }
        else if(query[0] == '5'){
            int fsp = query.find(' '), ssp = query.find(' ', fsp + 1);
            string dbname = query.substr(fsp + 1, ssp - fsp - 1);
            int tupleNum = stoi(query.substr(ssp + 1, query.size() - ssp));
            // cout << dbname << " " << tupleNum << endl;
            char* name = (char*) malloc(STRING * sizeof(char));
            char* meta = (char*) malloc(STRING * sizeof(char));
            strcpy(name, (dbname + ".schema").c_str());
            strcpy(meta, (dbname + ".meta").c_str());
            vector<pair<string,string>> schema = Read_Schema_File(name);
            strcpy(name, (dbname + ".db").c_str());
            Delete_Data_Tuple(name, meta, schema, tupleNum, Read_Tuple_Size(meta));
        }
        else if(query[0] == '6'){
            string dbname = "";
            int i = 2;
            while(i < (int)query.size() && query[i] != ' '){
                dbname += query[i];
                i++;
            }
            vector<int> arr;
            i++;
            string curr = "";
            while(i < (int)query.size()){
                if(query[i] == ' '){
                    arr.push_back(stoi(curr));
                    curr = "";
                }
                else curr += query[i];
                i++;
            }
            if(curr != "") arr.push_back(stoi(curr));
            char* name = (char*) malloc(STRING * sizeof(char));
            char* meta = (char*) malloc(STRING * sizeof(char));
            strcpy(meta, (dbname + ".meta").c_str());
            strcpy(name, (dbname).c_str());
            Update_Num_Index(meta, (int)arr.size());
            Write_Index(meta, arr);
            cout << dbname << endl;
            arr = Read_Index(meta);
            for(int i: arr) cout << i << " ";
            cout << endl;
            Initialize_DB(name, meta);
            InitializeIndexes();
        }
        else if(query[0] == '7'){
            string dbname = "";
            int i = 2;
            while(i < (int)query.size() && query[i] != ' '){
                dbname += query[i];
                i++;
            }
            i++;
            string curr = "";
            bool t = true;
            vector<pair<string,string>> arr;
            while(i < (int)query.size()){
                if(query[i] == ' '){
                    if(t) arr.push_back({ curr, "" });
                    else arr[(int)arr.size() - 1].second = curr;
                    curr = "";
                    t = !t;
                }
                else curr += query[i];
                i++;
            }
            if((int)arr.size() > 0) arr[(int)arr.size() - 1].second = curr;
            for(pair<string, string> p: arr){
                cout << p.first << " " << p.second << endl;
            }
            char* schname = (char*) malloc(STRING * sizeof(char));
            char* name = (char*) malloc(STRING * sizeof(char));
            char* meta = (char*) malloc(STRING * sizeof(char));
            char* temp = (char*) malloc(STRING * sizeof(char));
            strcpy(schname, (dbname + ".schema").c_str());
            strcpy(name, (dbname + ".db").c_str());
            strcpy(meta, (dbname + ".meta").c_str());
            strcpy(temp, (dbname).c_str());
            vector<pair<string, string>> schema = Read_Schema_File(schname);
            vector<int> blocks = Search_Database(temp, name, meta, schema, arr);
            cout << "Tuples" << endl;
            for(int j: blocks){
                vector<AttributeNode*> tuple = Read_Data_File(name, schema, j / getTupleSize(schema), getTupleSize(schema));
                PrintTuple(tuple);
                FreeTuple(tuple);
            }
        }
        else if(query[0] == '9'){
            cout << (int)names.size() << endl;
            for(int i = 0; i < (int)names.size(); i++){
                cout << "inorder for index: " << names[i] << endl;
                int root = Read_4Bytes_Address(names[i], 4);
                inorder(names[i], root);
            }
        }
        // else if(query[0] == '6'){
        //     char* name = (char*) malloc(STRING * sizeof(char));
        //     strcpy(name, "Student.avl");
        //     test1(name);
        // }
        // else if(query[0] == '7'){
        //     cout << "Enter key: " << endl;
        //     int key;
        //     cin >> key;
        //     char* name = (char*) malloc(STRING * sizeof(char));
        //     strcpy(name, "Student.avl");
        //     int root = Read_4Bytes_Address(name, 4);
        //     root = Delete_INT_AVLindex(name, root, -1, key);
        //     Update_Root_Address(name, root);
        // }
        // else if(query[0] == '8'){
        //     cout << "Inorder" << endl;
        //     char* name = (char*) malloc(STRING * sizeof(char));
        //     strcpy(name, "Student.avl");
        //     inorder(name, Read_4Bytes_Address(name, 4));
        // }
        // else if(query[0] == '9'){
        //     // test2();
        // }
    }
}