# ToDoL v1.0
The **T**o**D**o**L**ist

This is a basic terminal-based todo list program.
Use the presented command line to interact with your lists.

Unfortunately, there is no TUI at the moment. 
Interaction is done through the command line.

---
**Usage:**

add [list name]    : creates [list name]

remove [list name]    : deletes [list name]

rename [list name] [new name]    : renames [list name] to [new name]

list [list name] add [item name]    : adds [item name] to [list name]

list [list name] remove  [item name]    : removes [item name] from [list name]

list [list name] mark  [item name]    : marks [item name] from [list name]

list [list name] unmark  [item name]    : unmarks [item name] from [list name]

todo    : lists all lists

todo [list name]    : lists a specific list

help    : help

clear   : clears the screen

exit [-f]   : exit, [-f] forces exit without save
