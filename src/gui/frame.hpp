#ifndef FRAME_HPP
#define FRAME_HPP
#pragma once

#include <string>
#include <vector>
#include <utility>
#include <windows.h>

class CMDBox;
class CMDGrid;
class CMDFrame;

typedef struct RowData {
    uint32_t count;
    std::vector<uint32_t> rowheight;
} RowData;

typedef struct ColumnData {
    uint32_t count;
    std::vector<uint32_t> colwidth;
} ColumnData;

typedef struct Bordering {
    char topleft;
    char topright;
    char botmleft;
    char botmright;
    char topbody;
    char botmbody;
    char leftbody;
    char rightbody;
} Bordering;

typedef struct Indexing {
    int zindex;
    Indexing *next;
    Indexing *prev;
    std::vector<CMDBox*> members;
} Indexing;

typedef enum TextPosition
{
    TRUE_CENTER,
    CENTER_LEFT,
    CENTER_RIGHT,
    TOP_CENTER,
    TOP_LEFT,
    TOP_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_RIGHT,
    BOTTOM_CENTER
} TextPosition;

static inline int clearScreen() {
    // https://learn.microsoft.com/en-us/windows/console/clearing-the-screen
    HANDLE hStdOut;

    hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

    // Fetch existing console mode so we correctly add a flag and not turn off others
    DWORD mode = 0;
    if (!GetConsoleMode(hStdOut, &mode))
    {
        return ::GetLastError();
    }

    // Hold original mode to restore on exit to be cooperative with other command-line apps.
    const DWORD originalMode = mode;
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;

    // Try to set the mode.
    if (!SetConsoleMode(hStdOut, mode))
    {
        return ::GetLastError();
    }

    // Write the sequence for clearing the display.
    DWORD written = 0;
    PCWSTR sequence = L"\x1b[2J";
    if (!WriteConsoleW(hStdOut, sequence, (DWORD)wcslen(sequence), &written, NULL))
    {
        // If we fail, try to restore the mode on the way out.
        SetConsoleMode(hStdOut, originalMode);
        return ::GetLastError();
    }

    // To also clear the scroll back, emit L"\x1b[3J" as well.
    // 2J only clears the visible window and 3J only clears the scroll back.
    written = 0;
    sequence = L"\x1b[3J";
    if (!WriteConsoleW(hStdOut, sequence, (DWORD)wcslen(sequence), &written, NULL))
    {
        // If we fail, try to restore the mode on the way out.
        SetConsoleMode(hStdOut, originalMode);
        return ::GetLastError();
    }

    // Restore the mode on the way out to be nice to other command-line applications.
    SetConsoleMode(hStdOut, originalMode);

    return 0;
}

static inline void setCursor(uint32_t x = 0, uint32_t y = 0) {
    HANDLE handle;
    COORD coordinates;
    handle = GetStdHandle(STD_OUTPUT_HANDLE);
    coordinates.X = x;
    coordinates.Y = y;
    SetConsoleCursorPosition(handle, coordinates);
}

class CMDBox
{
    public:

        uint32_t width;                         // width of the box
        uint32_t height;                        // height of the box
        std::string name;                       // name of the box
        uint32_t posx = 0;                      // x-position of the box
        uint32_t posy = 0;                      // y-position of the box
        std::string inner;                      // inner text
        Bordering borders;                      // borders of the box
        bool visible = true;                    // visibility of box
        CMDBox *parent = NULL;                  // pointer to parent of box
        TextPosition textPosition = TOP_LEFT;   // position of the text

        /**
         * @brief Construct a new CMDBox object
         * 
         * @param nom Name of the box
         */
        CMDBox(std::string nom) : name(nom), height(10), width(10), inner(""), borders({0}) {}
        
        /**
         * @brief Construct a new CMDBox object
         * 
         * @param nom Name of the box
         * @param body Body of the box
         */
        CMDBox(std::string nom, std::string body) : name(nom), height(1), width(body.size()), inner(body), borders({0}) {}

        /**
         * @brief Construct a new CMDBox object
         * 
         * @param nom Name of the box
         * @param wid Width of the box
         * @param hig Height of the box
         */
        CMDBox(std::string nom, uint32_t wid, uint32_t hig) : name(nom), height(hig), width(wid), inner(""), borders({0})  {}

        /**
         * @brief Construct a new CMDBox object
         * 
         * @param nom Name of the box
         * @param wid Width of the box
         * @param hig Height of the box
         * @param body Body of the box
         */
        CMDBox(std::string nom, uint32_t wid, uint32_t hig, std::string body) : name(nom), height(hig), width(wid), inner(body), borders({0}) {}

        /**
         * @brief Get the character at a given position
         * 
         * @param x X-coordinate
         * @param y Y-coordinate
         * @return char at position `(x,y)`
         */
        virtual char getCharIn(uint32_t x, uint32_t y);

        /**
         * @brief Set all borders of the box
         * 
         * @param ch Border character
         */
        void setBorder(char ch);

        /**
         * @brief Set the bordered status
         * 
         * @param isBordered boolean indicating whether the table is to be bordered
         */
        virtual void setBordered(bool isBordered) {bordered = isBordered;}

        /**
         * @brief Get the bordered status
         * 
         * @return Current bordered status
         */
        virtual bool getBordered(bool isBordered) {return bordered;}

        /**
         * @brief Set the position of the box
         * 
         * @param x X-coordinate
         * @param y Y-coordinate
         * @param isRelative checks whether the position is relative or absolute
         */
        void setPosition(uint32_t x, uint32_t y, bool isRelative);

        /**
         * @brief Set the position of the box
         * 
         * @param pos Code marking the position of the box
         */
        void setPosition(TextPosition pos);

    protected:
        bool bordered = false;                  // bordered status of the box
};

class CMDFrame : public CMDBox
{
    public:

        std::string title;          // title of the frame

        /**
         * @brief Construct a new CMDFrame object
         * 
         */
        CMDFrame() : CMDBox("root") {}
        
        /**
         * @brief Construct a new CMDFrame object
         * 
         */
        CMDFrame(std::string nom) : CMDBox(nom) {}

        /**
         * @brief Construct a new CMDFrame object
         * 
         * @param wid Width of the frame
         * @param hig Height of the frame
         */
        CMDFrame(std::string nom, uint32_t wid, uint32_t hig) : CMDBox(nom, wid, hig) {}

        /**
         * @brief Construct a new CMDFrame object
         * 
         * @param nom Name of the frame
         * @param wid Width of the frame
         * @param hig Height of the frame
         * @param body Body text in the frame
         */
        CMDFrame(std::string nom, uint32_t wid, uint32_t hig, std::string body) : CMDBox(nom, wid, hig, body) {}

        /**
         * @brief Add child to frame
         * 
         * @param child Address of child to be added
         * @param zindex Z-index of child
         */
        void addChild(CMDBox *child, int zindex);

        /**
         * @brief Display the frame, and all of its contents
         * 
         */
        void display();

        /**
         * @brief Get child by name
         * 
         * @param nom Name of element to be gotten
         * @return CMDBox*, a pointer to the first element encountered with the name
         */
        CMDBox* getElementByName(std::string nom);

        /**
         * @brief Get the character at a given position
         * 
         * @param x X-coordinate
         * @param y Y-coordinate
         * @return char at position `(x,y)`
         */
        virtual char getCharIn(uint32_t x, uint32_t y) override;
    
    private:
        Indexing *children = NULL;
};

class CMDGrid : public CMDBox
{
    public:
        RowData rows;                   // table row data
        ColumnData columns;             // table column data
        char tableborderch;             // character used for table borders
        bool sizeByContents = false;    // row and column size adjustable when on
        
        /**
         * @brief Construct a new CMDGrid object
         * 
         * @param nom Name of the grid
         * @param r Number of rows in the grid
         * @param c Number of columns in the grid
         */
        CMDGrid(std::string nom, uint32_t r, uint32_t c) : CMDBox(nom, 0, 0) {
            // initialize row and col data (no nullptrs pls...)
            rows.count = 0;
            columns.count = 0;
            rows.rowheight = std::vector<uint32_t>();
            columns.colwidth = std::vector<uint32_t>();
            
            // initialize rows
            for (int i = 0; i < r; ++i) addRow();
            
            // initialize columns
            for (int i = 0; i < c; ++i) addColumn();
        }

        /**
         * @brief Construct a new CMDGrid object
         * 
         * @param nom Name of the grid
         * @param r Number of rows in the grid
         * @param c Number of columns in the grid
         * @param wid Width of each column
         * @param hig Height of each row
         */
        CMDGrid(std::string nom, uint32_t r, uint32_t c, uint32_t wid, uint32_t hig) : CMDGrid(nom, r, c) {
            // set height and width of grid
            for (int i = 0; i < rows.count; ++i) setHeight(i, hig);
            for (int i = 0; i < columns.count; ++i) setWidth(i, wid);
        }

        /**
         * @brief Add a row to the table.
         * 
         */
        void addRow();

        /**
         * @brief Add a column to the table.
         * 
         */
        void addColumn();

        /**
         * @brief Delete a row from the table.
         * 
         * @param row Index of the row.
         */
        void deleteRow(uint32_t row);

        /**
         * @brief Delete a column from the table.
         * 
         * @param col Index of the column.
         */
        void deleteColumn(uint32_t col);

        /**
         * @brief Set the height of the row of a table
         * 
         * @param row Index of the row
         * @param hig Height value to be added
         */
        void setHeight(uint32_t row, uint32_t hig);

        /**
         * @brief Set the Bordered object
         * 
         * @param isBordered boolean indicating whether the table is to be bordered
         */
        virtual void setBordered(bool isBordered) override;

        /**
         * @brief Set the width of the column of a table
         * 
         * @param col Index of the column
         * @param wid Width value to be added
         */
        void setWidth(uint32_t col, uint32_t wid);

        /**
         * @brief Get the character at a given position
         * 
         * @param x X-coordinate
         * @param y Y-coordinate
         * @return char at position `(x,y)`
         */
        virtual char getCharIn(uint32_t x, uint32_t y) override;

        /**
         * @brief Add child to Grid
         * 
         * @param child Address of child to be added
         * @param row Index of the row
         * @param col Index of the column
         * @param zindex Z-index of child
         */
        void addChild(CMDBox *child, int zindex, uint32_t row, uint32_t col);

        void adjustDimensions(uint32_t row, uint32_t col);

    private:
        std::vector<std::vector<CMDFrame*>> data;
};

#endif