//
// Created by Manoj Inukolunu on 11/10/25.
//

#include "SqlitePage.h"
#include "../common/Sqlite.h"

namespace table{
    void readTable(std::unique_ptr<btree::SqlitePage>& rootPage, Table table) {
        // 1. Load root page
        // 2. Get all cells , each cell is a row
        // 3. For Each cell
        //          read datatypes [(datatype),(datatype),(datatype),....]
        //          read content [(value),(value) , ....]
        // 4. Apply the schema after reading ,get the column name from the create table statement.
        // the index of the column is the same as the index of the content.
    }
}
