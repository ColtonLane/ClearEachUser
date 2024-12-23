#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>
#include <windows.h>
#include <Shobjidl.h>
#include <shellapi.h>

namespace fs = std::filesystem;
// "keepUsers" are the directories that are skipped over; users entered when prompted in mainLoop are added and passed over
std::vector <std::string> keepUsers = {"colto", "clane", "sday2", "bwhittenbarger", "Administrator", "templocal", "Default", "Public", "Default User", "All Users", "astambaugh", "romay", "aboggs3", "jwhitt2", "jsturm"}; 
std::vector <std::string> deleteQueue = {}; //vector of the users' AppData folders being deleted; is populated and cleared during runtime
std::string defaultUserPath = "C:/Users/"; 
std::string directoryPath; //updates to the user's entered path in mainLoop; is set to defaultUserPath is 'default' is typed
std::string disableHibernate = "powercfg.exe -h off"; //CMD command used to disable file duplicates created by Windows for use with hibernation (even if hibernation is unutilized on that computer)
double timeElapsed; //keeps amount of time elapsed in seconds
int convertToMB = 1000000; //factor to convert initialUserSpace and finalUserSpace from bytes to MB
int numUsersKept = 0; 
int numUsersDeleted = 0; 
int noAppData = 0; 


//adapted from https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
//Displays progress bar based on the number of AppData folders deleted out of the total number detected; Displays a timer as well
void progressBar(const int initialTotal) { 
    int deletedCount = 0;
    int cursorLocation = 0;
    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
    while (deleteQueue.size() > 0 || deletedCount < initialTotal) {
        //elapsed is updated each second to provide an accurate time from the start of this while loop until it completes; timeElapsed is a double that keeps the elapsed time in seconds
        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - startTime;
        timeElapsed = elapsed.count(); 

        deletedCount = initialTotal - deleteQueue.size();
        float progress = float(deletedCount) / initialTotal;
        int barWidth = 70;

        //The following code is printed to the bottom of the terminal and updated each second to provide a timer and constant progress bar
        //Either of these statements prints the time in the format "m:s"; adjusts according to number of seconds for proper display
        if (int(timeElapsed) % 60 < 10){
            std::cout << "(" << int(timeElapsed)/60 << ":" << "0"<< int(timeElapsed) % 60 << ") "; 
        }
        else{
            std::cout << "(" << int(timeElapsed)/60 << ":" << int(timeElapsed) % 60 << ") "; 
        }
        
        std::cout << deletedCount << " out of " << initialTotal << " deleted. ";
        //Code for printing the enclosed progress bar
        std::cout << "["; 
        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "|";
            else if (i == pos) std::cout << "_"; 
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(1000)); //Updates and re-prints each second
    }
    std::cout << std::endl; 
}

// Function to delete a folder in a separate thread, called from removeAppData
void deleteFolder(const fs::path& folderPath) {
    try {
        // Using the RD (Remove Directory) command to delete the folder
        std::string command = "RD /S /Q \"" + folderPath.string() + "\""; 
        deleteQueue.push_back(folderPath.string()); 
        int result = system(command.c_str()); // Execute the command
        if (result == 0) {
            numUsersDeleted++; 
        } else {
            //std::cerr << "Failed to delete " << folderPath << " with error code: " << result << std::endl; 
        }
        deleteQueue.erase(std::remove(deleteQueue.begin(), deleteQueue.end(), folderPath.string()), deleteQueue.end());
    } 
    catch (const std::exception& e) {
        //std::cerr << "Failed to delete " << folderPath << ": " << e.what() << std::endl;
    }
    
}

void removeAppData(fs::path& folderPath) {
    fs::path appDataPath = folderPath / "AppData";
    // Deletes AppData folder if one is found for that user; runs deleteFolder in new thread and detaches it for multi-threading
    if (fs::exists(appDataPath)) { 
        std::thread deleteThread(deleteFolder, appDataPath);
        deleteThread.detach(); // Detach the thread so it runs independently
    }
    // AppData folder not found; adds it to running tally to show user 
    else { 
        noAppData++; 
    }
}

int mainLoop() {
    std::string input; 
    std::cout << "Enter the path of the Users folder to be cleaned and press Enter (or type 'default' to use 'C:/Users/' as the path):" << std::endl;
    std::cin >> directoryPath;
    std::cout << "Enter the name(s) [not the path] of User(s) folders to be kept. Type 'done' when you've entered every folder name to keep:" << std::endl; 
    
    // Populates keepUsers with inputted Users' folder names; breaks when 'done' is detected
    while (input != "done") { 
        std::cin >> input; 
        if (input == "done") {
            break; 
        }
        keepUsers.push_back(input); 
    }

    for (int i = 0; i < directoryPath.length(); i++) {
        directoryPath[i] = tolower(directoryPath[i]);
    } 

    if (directoryPath == "default") {
        directoryPath = defaultUserPath; 
    }

    system(disableHibernate.c_str()); //disables hibernation and frees up the stored files hibernation uses

    try {
        // Iterate over the user directories; find and begin deletion for valid users' AppData
        for (auto& entry : fs::directory_iterator(directoryPath)) {
            fs::path p = entry.path(); 
            std::string userName = p.filename().string();

            //Check if the folder exists and is a directory; if so, calls removeAppData to delete the AppData folder
            if (fs::is_directory(p) && std::find(std::begin(keepUsers), std::end(keepUsers), userName) == std::end(keepUsers)) {
                if (fs::exists(p) && fs::is_directory(p)) {
                    removeAppData(p);  
                } 
                else {
                    std::cerr << "Skipping invalid directory: " << p.string() << std::endl;
                }
            }
             else {
                std::cout << "Kept: " << entry << std::endl; 
                numUsersKept++; 
            }
        }
    } 
    catch (const fs::filesystem_error& e) {
        std::cerr << "Filesystem error: " << e.what() << std::endl;
    } 
    catch (const std::exception& e) {
        std::cerr << "General exception: " << e.what() << std::endl;
    } 
    
    std::cout << std::endl; 
    return 0;
}

int main() {
    mainLoop(); 
    std::cout << "Number of Users kept: " << numUsersKept << std::endl;
    std::cout << "Number of Users without an AppData folder: " << noAppData << std::endl; 
    std::cout << "Deleting the rest of the Users' AppData folders. Keep this window open until that process completes. (some access errors may be thrown; these are normal and can be ignored)" << std::endl; 
    std::cout << std::endl; 

    //Case when no valid users to delete were found
    if (deleteQueue.size() < 1){
        std::cout << "No AppData folders to be deleted. Please check folder path and list of users to keep and try again." << std::endl; 
    }
    //Case when valid users to delete are found; runs the progressBar until the deletion queue is cleared
    else {
        int initialDelQueue = deleteQueue.size(); 
        progressBar(deleteQueue.size());
        std::cout << "Deleted " << initialDelQueue << " AppData folders in " << int(timeElapsed)/60 << "m " << int(timeElapsed)%60 << "s" << std::endl; 
    }

    std::cout << std::endl; 
    std::cout << "You may close the window or wait for it to close automatically" << std::endl; 
    std::this_thread::sleep_for(std::chrono::milliseconds(15000));
    return 0; 
}
