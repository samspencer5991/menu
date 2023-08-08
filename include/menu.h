/*
 * menu.h
 *
 *  Created on: 29 Jan 2020
 *      Author: samspencer
 */
#ifndef SRC_MENU_H_
#define SRC_MENU_H_

#include <stdint.h>

#define TRUE	1
#define FALSE 	0

#define MENU_LABEL_LENGTH 	26
#define MENU_MAX_LEVEL		7

typedef enum
{
	MenuMemError,
	MenuParamError,
	MenuOk
} MenuErrorStatus;

typedef enum
{
	Folder,
	Target
} ItemType;

/*
 * This is the structure for menu items. The item type can be determined by the following attributes
 * If there is a valid function assigned to the item, then it is a target not a folder (i.e. is actionable)
 * Otherwise, if it is NULL, the numChildren should be checked.
 * If there are 0 assigned children, it means the item is not populated, or a place holder folder.
 * If there are assigned children, the item is a folder and should be opened on selection.
 *
 */
struct MenuItem
{
	struct MenuItem **parent;				// pointer to parent level item (-1 level)
	struct MenuItem **child;				// pointer to child level items (+1 level) (FOLDER ONLY)
	uint8_t numChildren;						// number of child items held by this item (FOLDER ONLY)
	uint8_t numParents;							// Number of parent items that hold this child (for multi-parent items)
	void (*func)(uint8_t index);		// pointer to the function when target is chosen
	char name[MENU_LABEL_LENGTH+1];	// displayed string for the menu item
};

typedef struct MenuItem MenuItem;

typedef struct
{
	MenuItem **child;										// pointer for top level menu items
	MenuItem* currentFolder;						// currently accessed folder
	uint8_t numChildren;								// Number of top level child items in the menu system
	uint8_t currentIndex;								// currently accessed item of the current level
	uint8_t currentLevel;								// Currently accessd nested level
	uint8_t indexTrail[MENU_MAX_LEVEL];	// Previously entered indices
} MenuSystem;

typedef enum
{
	MenuNoAction,
	MenuNextChar,
	MenuPreviousChar,
	MenuNextSpace,
	MenuPreviousSpace,
	MenuConfirmText,
	MenuReturn,
	MenuExit
} MenuTextInputNav;

typedef enum
{
	MenuNoSelection,
	MenuConfirmSelection,
	MenuDeclineSelection,
} MenuSelectionChoice;

extern MenuTextInputNav menuTextNav;
extern MenuSelectionChoice menuSelectionChoice;
extern uint8_t menuCurrentInputIndex;
extern uint8_t menuCurrentInputChar;

void menu_NextItem(MenuSystem* menu);
void menu_PreviousItem(MenuSystem* menu);
void menu_SelectItem(MenuSystem* menu);
uint8_t menu_isTargetItem(MenuSystem* menu);
void menu_Back(MenuSystem* menu);
void menu_Open(MenuSystem* menu);
void menu_InitMenu(MenuSystem* menu);
MenuErrorStatus menu_InitItem(MenuItem* item, void (*func)(uint8_t index), const char name[]);
MenuErrorStatus menu_AssignTopChild(MenuSystem* menu, MenuItem* child);
MenuErrorStatus menu_AssignChild(MenuItem* parent, MenuItem* child);

#endif /* SRC_MENU_H_ */
