/*
 * menu.c
 *
 *  Created on: 29 Jan 2020
 *      Author: samspencer
 *
 *      A folder is an item which contains more items
 *      A target is an item contained in a folder which executes an action when selected
 *
 *      A target item has no sub-items. A folder can have both folder and target sub-items
 *
 *      Each item can only have one parent link,
 *      however, a parent may have multiple child links
 *      As a new child is assigned to an item, that item becomes both a parent and child
 *      This means a parent is always a folder, but a child may be a folder or a Target
 *
 *      The index property is used to order various elements within a folder
 */

#include "menu.h"
#include <stdlib.h>
#include <string.h>

MenuTextInputNav menuTextNav;
MenuSelectionChoice menuSelectionChoice;
uint8_t menuCurrentInputIndex;
uint8_t menuCurrentInputChar;

void menu_NextItem(MenuSystem* menu)
{
	// Check if the menu is on the top level or not as this changes the index check
	// Either way, increase the child item index, and wrap to 0 if overflowed
	if(menu->currentFolder == NULL)
	{
		if(menu->currentIndex == menu->numChildren - 1)
		{
			menu->currentIndex = 0;
		}
		else
		{
			menu->currentIndex++;
		}
	}
	else
	{
		if(menu->currentIndex == (menu->currentFolder->numChildren - 1))
		{
			menu->currentIndex = 0;
		}
		else
		{
			menu->currentIndex++;
		}
	}
}

void menu_PreviousItem(MenuSystem* menu)
{
	// Check if the menu is on the top level or not as this changes the index check
	// Either way, decrease the child item index, and wrap to size if overflowed
	if(menu->currentFolder == NULL)
	{
		if(menu->currentIndex == 0)
		{
			menu->currentIndex = menu->numChildren - 1;
		}
		else
		{
			menu->currentIndex--;
		}
	}
	else
	{
		if(menu->currentIndex == 0)
		{
			menu->currentIndex = menu->currentFolder->numChildren - 1;
		}
		else
		{
			menu->currentIndex--;
		}
	}
}

void menu_SelectItem(MenuSystem* menu)
{
	uint8_t index = menu->currentIndex;
	// If the menu is in the top level
	if(menu->currentFolder == NULL)
	{
		// If the child item is a folder and is not empty
		if(menu->child[index]->func == NULL && menu->child[index]->numChildren > 0)
		{
			// Update the selected child item as the new folder
			menu->currentFolder = menu->child[index];
			// Check if the child is a multi-parent item, and store the index before resetting it for later use
			// Update the indices
			menu->indexTrail[menu->currentLevel] = menu->currentIndex;
			menu->currentIndex = 0;
			menu->currentLevel++;
		}
		// If the child item is a target
		else if(menu->child[index]->func != NULL)
		{
			menu->child[index]->func(index);
		}
	}
	// If the menu is in a sub-level. The same logic applies as above, only with the child item as the currentFolder
	else
	{
		// If the child item is a folder and is not empty
		if(menu->currentFolder->child[index]->func == NULL && menu->currentFolder->child[index]->numChildren > 0)
		{

			// Check if the child is a multi-parent item,
			// store the index before resetting it for later use.
			// Also update the currentFolder parent with the parent index
			if(menu->currentFolder->child[index]->numParents > 1)
			{
				MenuItem* tempFolder = menu->currentFolder;
				// Update the selected child item as the new folder
				menu->currentFolder = menu->currentFolder->child[index];
				*menu->currentFolder->parent = tempFolder;
			}
			else
			{
				menu->currentFolder = menu->currentFolder->child[index];
			}
			// Update the indices
			menu->indexTrail[menu->currentLevel] = menu->currentIndex;
			menu->currentIndex = 0;
			menu->currentLevel++;
		}
		else if(menu->currentFolder->child[index]->func != NULL)
		{
			menu->currentFolder->child[index]->func(index);
		}
	}
}

uint8_t menu_isTargetItem(MenuSystem* menu)
{
	uint8_t index = menu->currentIndex;
	if(menu->currentFolder == NULL)
	{
		// If the child item is a target
		if(menu->child[index]->func != NULL)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if(menu->currentFolder->child[index]->func != NULL)
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
}

void menu_Back(MenuSystem* menu)
{
	// Catch the hard fault issue of a top level item going back
	if(menu->currentFolder == NULL)
	{
		return;
	}
	// If the current folder is a top level item
	if(menu->currentFolder->parent == NULL)
	{
		// When going back in the menu, the user should return to the previous index,
		// not just the previous level. By going through each child of the current Folder's parent,
		// and comparing each to the currentFolder, the index can be found
		for(int i=0; i<menu->numChildren; i++)
		{
			if(menu->child[i] == menu->currentFolder)
			{
				menu->currentIndex = i;
				break;
			}
		}
		menu->currentLevel--;
		menu->currentFolder = NULL;
	}

	// If the current folder is not a top level item
	// Prevent a hard fault by checking for the currentFolder being NULL (ie. already at top level)
	else if(menu->currentFolder != NULL)
	{
		menu->currentLevel--;
		menu->currentIndex = menu->indexTrail[menu->currentLevel];
		menu->currentFolder = *menu->currentFolder->parent;
	}
}

void menu_Open(MenuSystem* menu)
{
	menu->currentIndex = 0;
	menu->currentFolder = NULL;	// This is set to NULL to indicate that the current folder is the top menu folder
	menu->currentLevel = 0;
}

void menu_InitMenu(MenuSystem* menu)
{
	// Set all attributes to default values
	menu->child = NULL;
	menu->currentFolder = NULL;
	menu->numChildren = 0;
	menu->currentIndex = 0;
}

MenuErrorStatus menu_InitItem(MenuItem* item, void (*func)(uint8_t index), const char name[])
{
	// Check that the name is within the size restriction
	if(strlen(name) >= MENU_LABEL_LENGTH )
	{
		return MenuParamError;
	}
	// Set all attributes to default values
	item->parent = NULL;
	item->child = NULL;
	item->numChildren = 0;
	item->numParents = 0;
	item->func = func;
	strcpy(item->name, name);
	return MenuOk;
}

MenuErrorStatus menu_AssignTopChild(MenuSystem* menu, MenuItem* child)
{
	// newIndex is used to assign the index to the new item
	// Due to 0 indexing, this is used verbatim
	// e.g. if numChildren = 1, the index assigned will be 1 as there are now 2 items
	// By assigning the index before incrementing newChildren, 0 indexing is handled
	uint8_t newIndex = menu->numChildren;
	menu->numChildren++;

	// Set the parent to NULL so that the application knows it is a top level item
	child->parent = NULL;

	// If this is the first child to be assigned to the menu
	if(newIndex == 0)
	{
		menu->child = malloc(sizeof(MenuItem*));
		if(menu->child == NULL)
		{
			return MenuMemError;
		}
		menu->child[0] = child;
	}

	// If this is not the first child to be assigned to the menu
	else
	{
		menu->child = realloc(menu->child, sizeof(MenuItem*) * menu->numChildren);
		if(menu->child == NULL)
		{
			return MenuMemError;
		}
		menu->child[newIndex] = child;
	}
	return MenuOk;
}

MenuErrorStatus menu_AssignChild(MenuItem* parent, MenuItem* child)
{
	// newIndex is used to assign the index to the new item
	// Due to 0 indexing, this is used verbatim
	// e.g. if numChildren = 1, the index assigned will be 1 as there are now 2 items
	// By assigning the index before incrementing newChildren, 0 indexing is handled
	uint8_t newChildIndex = parent->numChildren;
	uint8_t newParentIndex = child->numParents;
	parent->numChildren++;
	child->numParents++;

	// Assign the parent/s to the child
	// If this is the first parent to be assigned to the child
	if(newParentIndex == 0)
		{
			child->parent = malloc(sizeof(MenuItem*));
			if(child->parent == NULL)
			{
				return MenuMemError;
			}
			child->parent[0] = parent;
		}

		// If this is not the first parent to be assigned to the child
		else
		{
			child->parent = (MenuItem**)realloc(child->parent, sizeof(MenuItem*) * child->numParents);
			if(child->parent == NULL)
			{
				return MenuMemError;
			}
			child->parent[newParentIndex] = parent;
		}

	// If this is the first child to be assigned to the parent
	if(newChildIndex == 0)
	{
		parent->child = malloc(sizeof(MenuItem*));
		if(parent->child == NULL)
		{
			return MenuMemError;
		}
		parent->child[0] = child;
	}

	// If this is not the first child to be assigned to the parent
	else
	{
		parent->child = (MenuItem**)realloc(parent->child, sizeof(MenuItem*) * parent->numChildren);
		if(parent->child == NULL)
		{
			return MenuMemError;
		}
		parent->child[newChildIndex] = child;
	}
	return MenuOk;
}
