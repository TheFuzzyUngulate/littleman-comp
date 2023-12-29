#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include "frame.hpp"

void CMDBox::setBorder(char ch)
{
    borders.topleft = ch;
    borders.topright = ch;
    borders.botmleft = ch;
    borders.botmright = ch;

    borders.topbody = ch;
    borders.botmbody = ch;
    borders.leftbody = ch;
    borders.rightbody = ch;
}

void CMDBox::setPosition(uint32_t x, uint32_t y, bool isRelative)
{
    posx = (isRelative && parent != NULL) ? parent->posx + x : x;
    posy = (isRelative && parent != NULL) ? parent->posy + y : y;
}

void CMDBox::setPosition(TextPosition pos)
{
    if (parent != NULL)
    {
        auto xmin = parent->posx;
        auto xmax = xmin + parent->width - 1;
        auto ymin = parent->posy;
        auto ymax = ymin + parent->height - 1;

        if (parent->bordered)
        {
            xmin += 1;
            xmax -= 1;
            ymin += 1;
            ymax -= 1;
        }

        auto xmid = ((xmax - xmin) / 2) + xmin;
        auto ymid = ((ymax - ymin) / 2) + ymin;

        switch (pos)
        {
            case TRUE_CENTER:
                posy = ymid - width/2;
                posx = xmid - width/2;
                break;

            case CENTER_LEFT:
                posy = ymid - width/2;
                posx = xmin;
                break;

            case CENTER_RIGHT:
                posy = ymid - width/2;
                posx = xmax - width + 1;
                break;

            case TOP_CENTER:
                posy = ymin;
                posx = xmid - width/2;
                break;

            case TOP_LEFT:
                posy = ymin;
                posx = xmin;
                break;

            case TOP_RIGHT:
                posy = ymin;
                posx = xmax - width + 1;
                break;
                
            case BOTTOM_CENTER:
                posy = ymax - height + 1;
                posx = xmid - width/2;
                break;
                
            case BOTTOM_LEFT:
                posy = ymax - height + 1;
                posx = xmin;
                break;
                
            case BOTTOM_RIGHT:
                posy = ymax - height + 1;
                posx = xmax - width + 1;
                break;
        }
    }
}

char CMDBox::getCharIn(uint32_t x, uint32_t y)
{
    // out of bounds returns 0
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;

    // check if border falls in coords
    if (bordered)
    {
        // corners
        if (x == 0 && y == 0) return borders.topleft;
        if (x == 0 && y == height-1) return borders.botmleft;
        if (x == width-1 && y == 0) return borders.topright;
        if (x == width-1 && y == height-1) return borders.botmright;

        // body
        if (x == 0) return borders.leftbody;
        if (x == width-1) return borders.rightbody;
        if (y == 0) return borders.topbody;
        if (y == height-1) return borders.botmbody;
    }

    // check if text falls in coords
    uint32_t minx = 0, miny = 0;
    // length of inner text
    uint32_t len = inner.length();

    switch (textPosition)
    {
        case CENTER_LEFT:
            miny = height/2;
            minx = bordered ? 1 : 0;
            break;

        case CENTER_RIGHT:
            miny = height/2;
            minx = width-1-len - (bordered ? 1 : 0);
            break;

        case TRUE_CENTER:
            miny = height/2;
            minx = width/2 - len/2;
            break;

        case TOP_CENTER:
            miny = bordered ? 1 : 0;
            minx = width/2 - len/2;
            break;

        case TOP_LEFT:
            miny = bordered ? 1 : 0;
            minx = bordered ? 1 : 0;
            break;

        case TOP_RIGHT:
            miny = bordered ? 1 : 0;
            minx = width-1-len - (bordered ? 1 : 0);
            break;
            
        case BOTTOM_CENTER:
            miny = bordered ? height-2 : height-1;
            minx = width/2 - len/2;
            break;
            
        case BOTTOM_LEFT:
            miny = bordered ? height-2 : height-1;
            minx = bordered ? 1 : 0;
            break;
            
        case BOTTOM_RIGHT:
            miny = bordered ? height-2 : height-1;
            minx = width-1-len - (bordered ? 1 : 0);
            break;
    }

    // check if coords falls in inner
    if (y == miny && (x >= minx && x < minx + len)) return inner.at(x - minx);

    return 0;
}

char CMDFrame::getCharIn(uint32_t x, uint32_t y)
{
    auto ch = CMDBox::getCharIn(x, y);

    for (auto c_set = children; c_set != NULL && c_set->zindex >= 0; c_set = c_set->next)
    {
        bool found = false;
        for (auto box : c_set->members) 
        {
            char nch;

            if (static_cast<CMDFrame*>(box) != nullptr)
                nch = ((CMDFrame*)box)->getCharIn(x - box->posx, y - box->posy);
            else 
            if (static_cast<CMDGrid*>(box) != nullptr)
                nch = ((CMDGrid*)box)->getCharIn(x - box->posx, y - box->posy);
            else 
                nch = box->getCharIn(x - box->posx, y - box->posy);
            
            if (nch != 0) 
            {
                ch = nch;
                found = true;
                break;
            }
        }

        if (found) break;
    }

    return ch;
}

CMDBox* CMDFrame::getElementByName(std::string nom)
{
    for (auto c_set = children; c_set != NULL && c_set->zindex >= 0; c_set = c_set->next)
    {
        for (auto box : c_set->members) 
        {
            if (box->name == nom) return box;
        }
    }

    return NULL;
}

void CMDFrame::display() 
{
    for (int i = 0; i < posy; ++i)
        std::cout << std::endl;

    for (int y = 0; y < height; ++y)
    {
        // space out
        std::cout << std::string(posx, ' ');

        // handle each char
        for (int x = 0; x < width; ++x)
        {
            // attempt getCharIn on yourself
            char ch = getCharIn(x, y);

            if (ch == 0) 
                ch = ' ';
            printf("%c", ch);
        }
        
        printf("\n");
    }

}

void CMDFrame::addChild(CMDBox *child, int zindex) 
{
    child->parent = this;
    auto level = children;

    if (level != NULL)
    {
        if (zindex < level->zindex) {
            while (level->next != NULL && zindex < level->zindex)
                level = level->next;
            if (level->zindex == zindex) {
                level->members.push_back(child);
            }
            else
            {
                auto nlevel = new Indexing;
                nlevel->next = level;
                nlevel->prev = level->prev;
                level->prev = nlevel;
                nlevel->zindex = zindex;
                nlevel->members.push_back(child);
            }
        }
        else
        if (zindex > level->zindex) {
            auto nlevel = new Indexing;
            nlevel->next = level;
            nlevel->prev = NULL;
            level->prev = nlevel;
            nlevel->zindex = zindex;
            nlevel->members.push_back(child);
            return;
        }
        else level->members.push_back(child);
    }
    else 
    {
        children = new Indexing;
        children->next = NULL;
        children->prev = NULL;
        children->zindex = zindex;
        children->members.push_back(child);
    }
}

void CMDGrid::addRow()
{
    rows.count++;
    rows.rowheight.push_back(0);
    
    data.push_back(std::vector<CMDFrame*>());
    for (int x = 0; x < columns.count; ++x) 
    {
        auto nFrame = new CMDFrame("", columns.colwidth[x], rows.rowheight[rows.count-1]);
        nFrame->parent = this;
        nFrame->posx = (bordered) ? posx + 1 : posx;                        // border is one extra character
        nFrame->posy = height + posy;                                       // height counts borders, so no adjustments necessary
        data[rows.count-1].push_back(nFrame);
    }
}

void CMDGrid::addColumn()
{
    columns.count++;
    columns.colwidth.push_back(0);

    for (int y = 0; y < rows.count; ++y)
    {
        auto nFrame = new CMDFrame("", columns.colwidth[columns.count-1], rows.rowheight[y]);
        nFrame->parent = this;
        nFrame->posx = width + posx;                                        // width counts border, so no adjustments necessary
        nFrame->posy = (bordered) ? posy + 1 : posy;                        // border is on extra character
        data[y].push_back(nFrame);
    }
}

void CMDGrid::deleteRow(uint32_t row)
{
    if (row < rows.count)
    {
        // adjust height
        auto rh = rows.rowheight[row];
        height -= rh;

        // account for border thickness
        if (bordered)
            height -= 1 + (rows.count < 2 ? 1 : 0);

        // delete row
        auto datum = data[row];
        data.erase(data.begin() + row);
        // delete all column entries in row
        for (int i = 0; i < columns.count; ++i)
            delete datum[i];

        // adjust row count
        rows.count--;
        // delete records of rowheight in rows
        rows.rowheight.erase(rows.rowheight.begin() + row);

        // adjust posy for remaining entries
        for (int y = row; y < rows.count; ++y)
            for (int x = 0; x < columns.count; ++x)
                data[y][x]->posy -= rh + (bordered ? 1 : 0);
    }
}

void CMDGrid::deleteColumn(uint32_t col)
{
    if (col < columns.count)
    {
        // adjust width
        auto cw = columns.colwidth[col];
        width -= cw;

        // account for border thickness
        if (bordered)
            width -= 1 + (columns.count < 2 ? 1 : 0);

        // delete column entries at col in all rows
        for (auto row : data)
        {
            auto datum = row[col];
            row.erase(row.begin() + col);
            delete datum;
        }

        // adjust column count
        columns.count--;
        // delete records of colwidth in rows
        columns.colwidth.erase(columns.colwidth.begin() + col);

        // adjust posx for remaining entries
        for (int y = 0; y < rows.count; ++y)
            for (int x = col; x < columns.count; ++x)
                data[y][x]->posx -= cw + (bordered ? 1 : 0);
    }
}

void CMDGrid::setHeight(uint32_t row, uint32_t hig)
{
    if (row < rows.count)
    {
        // adjust height of grid
        height += hig;
        height -= rows.rowheight[row];

        // account for border thickness
        if (bordered)
        {
            // if setting to zero from non-zero, remove borders
            if (hig == 0 && rows.rowheight[row] > 0)
                height -= 1 + (rows.count < 2 ? 1 : 0);

            // if setting to non-zero from zero, add borders
            if (hig > 0 && rows.rowheight[row] == 0)
                height += 1 + (rows.count < 2 ? 1 : 0);
        }

        // adjust height of all cells in row
        for (int i = 0; i < columns.count; ++i)
            data[row][i]->height = hig;

        // adjust posy for all lower cells
        for (int y = row + 1; y < rows.count; ++y) 
        {
            for (int x = 0; x < columns.count; ++x)
            {
                data[y][x]->posy += hig;
                data[y][x]->posy -= rows.rowheight[row];
            }
        }

        // update record
        rows.rowheight[row] = hig;
    }
}

void CMDGrid::setWidth(uint32_t col, uint32_t wid)
{
    if (col < columns.count)
    {
        // adjust width of grid
        width += wid;
        width -= columns.colwidth[col];

        // account for border thickness
        if (bordered)
        {
            // if setting to zero from non-zero, remove borders
            if (wid == 0 && columns.colwidth[col] > 0)
                width -= 1 + (columns.count < 2 ? 1 : 0);

            // if setting to non-zero from zero, add borders
            if (wid > 0 && columns.colwidth[col] == 0)
                width += 1 + (columns.count < 2 ? 1 : 0);
        }

        // adjust width of all cells in column
        for (int i = 0; i < rows.count; ++i)
            data[i][col]->width = wid;

        // adjust posx for all lower cells
        for (int y = 0; y < rows.count; ++y) 
        {
            for (int x = col + 1; x < columns.count; ++x)
            {
                data[y][x]->posx += wid;
                data[y][x]->posx -= columns.colwidth[col];
            }
        }

        // update record
        columns.colwidth[col] = wid;
    }
}

void CMDGrid::addChild(CMDBox *child, int zindex, uint32_t row, uint32_t col)
{
    if (row < rows.count && col < columns.count)
    {
        // add child to correct cell
        auto cell = data[row][col];
        child->parent = cell;
        cell->addChild(child, zindex);

        // adjust cell size to accommodate child (add a parameter that adjusts this)
        if (sizeByContents)
        {
            // adjust row height based on maximum height in that row
            // adjust column width based on maximum width in that column
            for (int y = 0; y < rows.count; ++y)
            {
                for (int x = 0; x < columns.count; ++x)
                {
                    if (data[y][x]->height > rows.rowheight[y])
                        rows.rowheight[y] = data[y][x]->height;
                    if (data[y][x]->width > columns.colwidth[x])
                        columns.colwidth[x] = data[y][x]->width;
                }
            }
        }
    }
}

char CMDGrid::getCharIn(uint32_t x, uint32_t y)
{
    // out of bounds returns 0
    if (x < 0 || x >= width || y < 0 || y >= height) return 0;

    // check if table border
    if (bordered)
    {
        for (int ty = 0; ty < rows.count; ++ty) 
        {
            for (int tx = 0; tx < columns.count; ++tx)
            {
                // check if x = cell->posx - 1 for any cells (left border)
                if (x == (data[ty][tx]->posx - 1)) return tableborderch;
                // check if y = cell->posy - 1 for any cells (top border)
                if (y == (data[ty][tx]->posy - 1)) return tableborderch;
                // check if x = cell->posx + cell->width for any cells (right border)
                if (x == (data[ty][tx]->posx + data[ty][tx]->width)) return tableborderch;
                // check if y = cell->posy + cell->height for any cells (bottom border)
                if (y == (data[ty][tx]->posy + data[ty][tx]->height)) return tableborderch;
            }
        }
    }

    // otherwise, must be a cell
    // find cell to add in (calculate based on loop)
    for (int ty = 0; ty < rows.count; ++ty) 
    {
        for (int tx = 0; tx < columns.count; ++tx)
        {
            // if quadrants land within square, then calculate recursively

            auto xmin = data[ty][tx]->posx;
            auto xmax = xmin + data[ty][tx]->width - 1;
            auto ymin = data[ty][tx]->posy;
            auto ymax = ymin + data[ty][tx]->height - 1;

            if (xmin <= x && x <= xmax && ymin <= y && y <= ymax)
                return data[ty][tx]->getCharIn(x - xmin, y - ymin);
        }
    }

    return 0;                       // added to assuage C++'s semchecker
}

void CMDGrid::setBordered(bool isBordered)
{
    std::cout << "dimensions: (" << width << " x " << height << ")\n";

    // do nothing if status is same
    if (bordered == isBordered)
        return;
    
    // otherwise, assume they are different
    // adjust posx and posy, as well as width and height, accordingly
    if (isBordered)
    {
        for (int y = 0; y < rows.count; ++y)
        {
            for (int x = 0; x < columns.count; ++x)
            {
                data[y][x]->posx += x + 1;
                data[y][x]->posy += y + 1;
            }
        }

        height += rows.count + 1;
        width += columns.count + 1;
    }
    else
    {
        for (int y = 0; y < rows.count; ++y)
        {
            for (int x = 0; x < columns.count; ++x)
            {
                data[y][x]->posx -= x + 1;
                data[y][x]->posy -= y + 1;
            }
        }

        height -= rows.count + 1;
        width -= columns.count + 1;
    }

    std::cout << "dimensions: (" << width << " x " << height << ")\n";

    bordered = isBordered;
}

int main()
{
    //clearScreen();

    // init
    CMDFrame frame = CMDFrame();
    frame.width = 19;
    frame.height = 19;
    frame.setBorder('+');
    frame.setBordered(true);
    frame.textPosition = TRUE_CENTER;

/*
    CMDBox *clock = new CMDBox("clock", 9, 3, "00:00");
    frame.addChild(clock, 1);
    clock->bordered = true;
    clock->setBorder('*');
    clock->textPosition = TRUE_CENTER;
    clock->setPosition(TRUE_CENTER);

    CMDFrame *mainMenu = new CMDFrame("main_menu", 63, 4);
    frame.addChild(mainMenu, 1);
    mainMenu->bordered = true;
    mainMenu->borders.topbody = '-';
    mainMenu->setPosition(BOTTOM_CENTER);

    CMDBox *invenButton = new CMDBox("inventory", "inventory");
    mainMenu->addChild(invenButton, 1);
    invenButton->width = 13;
    invenButton->height = 3;
    invenButton->bordered = true;
    invenButton->setBorder('*');
    invenButton->textPosition = TRUE_CENTER;
    invenButton->setPosition(TRUE_CENTER);
*/

    CMDGrid *table = new CMDGrid("grid1", 3, 3, 5, 5);
    frame.addChild(table, 0);
    table->tableborderch = '*';
    table->setBordered(true);
    table->setPosition(TOP_CENTER);

    // display
    frame.display();
}