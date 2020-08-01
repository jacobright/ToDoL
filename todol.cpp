#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <fstream>
#include <sys/ioctl.h>

using namespace std;

#define BOX_OPEN "[ ]"
#define BOX_TICKED "[X]"
#define SAVE_FILE "todol_lists.txt"
#define HEADER "ToDoL v1.0"

struct todoitem {
  string name;
  bool ticked;
};

struct todolist {
    string name;
    vector<struct todoitem> list;
};

struct winsize size;

// the global list of todolists
vector<struct todolist> lists;

// setANSI: used mostly as a helper function for printMessage, sets the ANSI code
void setANSI(string code) {
  cout << "\e" << code;
}

void clearLine();

// printMessage: an easier way for me to print colored text
void printMessage(string msg, string textColor, string bgColor) {
  clearLine();
  setANSI(textColor);
  setANSI(bgColor);
  cout << msg;
  setANSI("[0m");
  cout << endl;
}

void clearLine() {
  cout << "\e[K";
}

void clearScreen(bool showHeader) {
  cout << "\e[0;0H";
  for (int i = 0; i < size.ws_row-1; i++) {
    cout << "\e[K" << endl;
  }
  cout << "\e[0;0H";
  if (showHeader)
    printMessage(HEADER, "[30m", "[45m");
}

// tokenize: returns the input, but broken into a vector of strings
vector<string> tokenize(string input) {
    vector<string> tokens;
    stringstream stream(input);
    string token;

    while (stream >> token) {
        tokens.push_back(token);
    }

    return tokens;
}

// untokenizeFromCommand: turns a vector back into a string, used for some commands
string untokenizeFromCommand(vector<string> args) {
  string str = "";

  for (int i = 1; i < args.size()-1; i++) {
    str += args[i] + " ";
  }
  str += args[args.size()-1];

  return str;
}
string untokenizeFromCommand(vector<string> args, int start) {
  string str = "";

  for (int i = start; i < args.size()-1; i++) {
    str += args[i] + " ";
  }
  str += args[args.size()-1];

  return str;
}

// printList: prints the specified todo list and it's contents
void printList(string listName) {
  int i = -1;

  for (int n = 0; n < lists.size(); n++) {
    if (lists[n].name.compare(listName) == 0) {
      i = n;
      break;
    }
  }

  if (i == -1) {
    printMessage("list \'" + listName + "\' does not exist.", "[31m", "[49m");
    return;
  }

  printMessage(lists[i].name, "[30m", "[46m");// << endl;
    
  setANSI("[36m");
  for(int j = 0; j < lists[i].list.size(); j++) {
    if (lists[i].list[j].ticked)
      cout << BOX_TICKED << " ";
    else
      cout << BOX_OPEN << " ";

    cout << lists[i].list[j].name << endl;
  }
  setANSI("[0m");
}

// printLists: prints all todo lists and their contents
void printLists() {
  int curLine = 0;
  int maxLine = size.ws_row-1;
  for (int i = curLine; i < lists.size(); i++) {
    //lists[i].list.size();

    printList(lists[i].name);
    cout << endl;
  }
}

// makeList: creates a todo list with the name specified
void makeList(vector<string> args) {
  struct todolist l;// = new struct todolist;//;new struct todolist l;
  string n = args[1];

  // Check if the list exists already, let the user know if it does, and don't create a new list 
  for (int i = 0; i < lists.size(); i++) {
    if (lists[i].name.compare(n) == 0) {
      printMessage("list \'" + n + "\' exists already.", "[31m", "[49m");
      return;
    }
  }
  l.name = n;

  lists.push_back(l);
  // cout << "created list \'" << n << "\'" << endl;
  printMessage("created list \'" + n + "\'", "[32m", "[49m");
}

// removeList: removes the todo list with the name specified
void removeList(vector<string> args) {
  string n = args[1];

  // remove all lists if listname is "*"
  if (n.compare("*") == 0) {
    for (int i = 0; i < lists.size(); i++) {
      lists[i].list.clear();
    }
    lists.clear();

    printMessage("removed all lists", "[32m", "[49m");
    return;
  }

  for (int i = 0; i < lists.size(); i++) {
    if (lists[i].name.compare(n) == 0) {
      lists[i].list.clear();
      lists.erase(lists.begin() + i);
      printMessage("removed list \'" + n + "\'", "[32m", "[49m");
      return;
    }
  }

  printMessage("list \'" + n + "\' does not exist.", "[31m", "[49m");
}

// renameList: replaces the specified list's name with the new name
void renameList(vector<string> args) {
  string n = args[1];
  string n2 = args[2];

  for (int i = 0; i < lists.size(); i++) {
    if (lists[i].name.compare(n) == 0) {
      lists[i].name = n2;
      printMessage("renamed list \'" + n + "\' to \'" + n2 + "\'", "[32m", "[49m");
      return;
    }
  }

  printMessage("list \'" + n + "\' does not exist.", "[31m", "[49m");
}

// addToList: adds items to todo lists
void addToList(vector<string> args) {
  string listName = args[1];
  string itemName = untokenizeFromCommand(args, 3);

  struct todoitem item;
  
  item.name = itemName;
  item.ticked = false;

  for (int i = 0; i < lists.size(); i++) {
    if (lists[i].name.compare(listName) == 0) {
      lists[i].list.push_back(item);
      printMessage("added \'" + itemName + "\' to " + listName, "[32m", "[49m");
      return;
    }
  }

  printMessage("list \'" + listName + "\' does not exist.", "[31m", "[49m");
}

// removeFromList: removes items from todo lists
void removeFromList(vector<string> args) { 
  string listName = args[1];
  string itemName = untokenizeFromCommand(args, 3);

  // find the list
  for (int i = 0; i < lists.size(); i++) {
    if (lists[i].name.compare(listName) == 0) {
      if (itemName.compare("*") == 0) { // if '*' is itemName, remove EVERYTHING from list
        lists[i].list.clear();
        printMessage("removed all items from \'" + listName + "\'", "[32m", "[49m");
        return;
      }

      //find the item
      for(int j = 0; j < lists[i].list.size(); j++) {
        if (lists[i].list[j].name.compare(itemName) == 0) {
          lists[i].list.erase(lists[i].list.begin() + j);
          printMessage("removed \'" + itemName + "\' from " + listName, "[32m", "[49m");
          return;
        }
      }
      printMessage("item \'" + itemName + "\' does not exist in list \'" + listName + "\'", "[31m", "[49m");
      return;
    }
  }

  printMessage("list \'" + listName + "\' does not exist.", "[31m", "[49m");
}

// removeFromList: removes items from todo lists
void setMark(vector<string> args, bool mark) { 
  string listName = args[1];
  string itemName = untokenizeFromCommand(args, 3);

  // find the list
  for (int i = 0; i < lists.size(); i++) {
    if (lists[i].name.compare(listName) == 0) {
      //find the item
      for(int j = 0; j < lists[i].list.size(); j++) {
        if (itemName.compare("*") == 0) { // if '*' is itemName, remove EVERYTHING from list
          lists[i].list[j].ticked = mark;
          if (mark)
            printMessage("marked all items from \'" + listName + "\'", "[32m", "[49m");
          else
            printMessage("unmarked all items from \'" + listName + "\'", "[32m", "[49m");
        }
        else if (lists[i].list[j].name.compare(itemName) == 0) {
          lists[i].list[j].ticked = mark;
          if (mark)
            printMessage("marked \'" + itemName + "\'", "[32m", "[49m");
          else
            printMessage("unmarked \'" + itemName + "\'", "[32m", "[49m");

          return;
        } 

      }
      printMessage("item \'" + itemName + "\' does not exist in list \'" + listName + "\'", "[31m", "[49m");
      return;
    }
  }

  printMessage("list \'" + listName + "\' does not exist.", "[31m", "[49m");
}


// saveLists: saves the lists you've created to a file
void saveLists() {
  ofstream saveFile(SAVE_FILE);

  for (int i = 0; i < lists.size(); i++) {
    saveFile << "=" << lists[i].name << endl;
    for (int j = 0; j < lists[i].list.size(); j++) {

      if (lists[i].list[j].ticked == true)
        saveFile << 1;
      else
        saveFile << 0;

      saveFile << lists[i].list[j].name << endl;
    }
  }
  printMessage("Saved!", "[32m", "[49m");
  saveFile.close();
}

// loadLists: loads the lists you've saved to a file
void loadLists() {
  ifstream saveFile(SAVE_FILE);
  string line;
  int whichList = -1;

  if (saveFile.is_open())
  {
    vector<string> fakeCmd;

    fakeCmd = {"rm", "*"};
    removeList(fakeCmd);
    fakeCmd.clear();

    while (getline (saveFile,line) )
    {
      // if it's a list title
      if (line[0] == '=') {
        fakeCmd = {"add", line.substr(1,line.length()-1)};
        makeList(fakeCmd);
        whichList++;
      }
      else if (whichList >= 0) {
        // if item is unticked
        if (line[0] == '0') {
          struct todoitem newitem;
          newitem.name = line.substr(1);
          newitem.ticked = false;
          lists[whichList].list.push_back(newitem);
        }
        // if item is ticked
        else if (line[0] == '1') {
          struct todoitem newitem;
          newitem.name = line.substr(1);
          newitem.ticked = true;
          lists[whichList].list.push_back(newitem);
        }
      }
      else {
        printMessage("Corrupted Save File!", "[31m", "[49m");
        saveFile.close();
        break;
      }
    }
    saveFile.close();
  }
  else {
    printMessage("No Save File to load from!", "[31m", "[49m");
  }
}



// main: takes input and processes commands
int main() {
  string input;
  vector<string> command;
  ioctl(STDOUT_FILENO, TIOCGWINSZ, &size);

  cout << "\e[2J" << "\e[0;0H";
  loadLists();
  clearScreen(true);

  while(1) {
    clearLine();
    cout << ">>";
    cout << "\e[s" << "\e[0;0H";
    printMessage(HEADER, "[30m", "[45m");
    cout << "\e[u";

    getline(cin, input);
    command = tokenize(input);

    if (command.size() == 0) {
      continue;
    }

    // HERE ARE THE COMMANDS
    if (command[0] == "add" || command[0] == "a") { // makes a list (CHANGE)
      if (command.size() > 1) {
        makeList(command);
      }
      else {
        printMessage("add [list name]", "[31m", "[49m");
      }
    }
    else if (command[0] == "remove" || command[0] == "rm") { // removes a list (CHANGE)
      if (command.size() > 1) {
        removeList(command);
      }
      else {
        printMessage("remove [list name]", "[31m", "[49m");
      }
    }
    else if (command[0] == "rename" || command[0] == "rn") {
      if (command.size() != 3) {
        printMessage("rename [list name] [new name]", "[31m", "[49m");
      }
      else {
        renameList(command);
      }
    }
    else if (command[0] == "list" || command[0] == "ls") { // actions on a list
      if (command.size() < 4) {
        printMessage("list [list name] add [item name]", "[31m", "[49m");
        printMessage("list [list name] remove [item name]", "[31m", "[49m");
        printMessage("list [list name] mark [item name]", "[31m", "[49m");
        printMessage("list [list name] unmark [item name]", "[31m", "[49m");
        continue;
      }

      if (command[2] == "add" || command[2] == "a") { // adds item to list
        addToList(command);
      }
      else if (command[2] == "remove" || command[2] == "rm") { // removes item from list
        removeFromList(command);
      }
      else if (command[2] == "mark" || command[2] == "m") { // removes item from list
        setMark(command, true);
      }
      else if (command[2] == "unmark" || command[2] == "um") { // removes item from list
        setMark(command, false);
      }
      else {
        printMessage("list [list name] add [item name]", "[31m", "[49m");
        printMessage("list [list name] remove [item name]", "[31m", "[49m");
        printMessage("list [list name] mark [item name]", "[31m", "[49m");
        printMessage("list [list name] unmark [item name]", "[31m", "[49m");
      }
    }
    else if (command[0] == "todo" || command[0] == "td" || command[0] == "t") { // prints all of your todo lists
      if (lists.size() == 0) {
        printMessage("No lists to print", "[31m", "[49m");
        continue;
      }

      clearScreen(true);

      if (command.size() == 2) {
        printList(command[1]);
      }
      else {
        printLists();
      }
    }
    else if (command[0] == "help") {
      clearScreen(true);
      printMessage("add [list name]"
                   "\nremove [list name]"
                   "\nrename [list name] [new name]"
                   "\nlist [list name] add [item name]"
                   "\nlist [list name] remove  [item name]"
                   "\nlist [list name] mark  [item name]"
                   "\nlist [list name] unmark  [item name]"
                   "\ntodo"
                   "\ntodo [list name]"
                  "\nhelp"
                  "\nclear"
                  "\nexit [-f]",
                  "[35m", "[49m");
    }
    else if (command[0] == "clear" || command[0] == "cls") { // clears the screen (TODO: REMOVE)
      clearScreen(true);
    }
    else if (command[0] == "exit") {
      clearScreen(true);
      if (command.size() > 1) {
        if (command[1] != "-f") {
          saveLists();
        }
      }
      else {
        saveLists();
      }

      printMessage("Exiting...", "[35m", "[49m");
      usleep(250000);
      break;
    }
    else {
        printMessage("Unknown command \'" + command[0] + "\'", "[31m", "[49m");
    }
  }

  //TODO: ASSIGN ID NUBMERS TO EACH LIST FOR EASIER COMMAND EDITING
  //TODO: SCROLLING TODOLISTS IF TOO LONG
  //TODO: SPLIT INTO MULTIPLE FILES FOR CRYING OUT LOUD

  //MAYBETODO:
  //CURSES: display todolists on side instead
    //Keyboard shortcuts & control
    //'Edit' command to graphically edit the list

  clearScreen(false);
  return 0;
}
