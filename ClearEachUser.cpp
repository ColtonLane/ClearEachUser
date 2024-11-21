// #include <iostream>
// #include <filesystem>
// #include <vector>
// #include <algorithm>
// #include <string>
// #include <chrono>
// #include <thread>
// #include <windows.h>
// #include <Shobjidl.h>
// #include <shellapi.h>

// namespace fs = std::filesystem;

// // "keepUsers" are the directories that are skipped over
// std::vector <std::string> keepUsers = {"colto", "clane", "sday2", "bwhittenbarger", "Administrator", "templocal", "Default", "Public", "Default User", "All Users", "astambaugh", "romay", "aboggs3", "jwhitt2", "jsturm"}; 
// std::vector <std::string> deleteQueue = {}; //vector of the users' AppData folders being deleted; is populated and cleared during runtime

// std::string defaultUserPath = "C:/Users/"; 
// std::string directoryPath; //updates to the user's entered path in mainLoop

// double startingSpace; //keeps the amount of space the system starts with (kept in MBs by the processes that update it)
// std::filesystem::space_info info; //space_info variable to keep track of starting and completed deletion space
// double timeElapsed; 

// int bytesToMB = 1000000.0; 
// int numUsersKept = 0; 
// int numUsersDeleted = 0; 
// int noAppData = 0; 

// void progressBar(const int initialTotal) { //adapted from https://stackoverflow.com/questions/14539867/how-to-display-a-progress-indicator-in-pure-c-c-cout-printf
//     int deletedCount = 0;
//     int cursorLocation = 0;
//     std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();
//     while (deleteQueue.size() > 0 || deletedCount < initialTotal) {
//         std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - startTime;
//         timeElapsed = elapsed.count(); 
//         deletedCount = initialTotal - deleteQueue.size();
//         float progress = float(deletedCount) / initialTotal;
//         int barWidth = 70;
//         std::cout << "(" << int(timeElapsed)/60 << ":" << int(timeElapsed) % 60 << ") "; 
//         std::cout << deletedCount << " out of " << initialTotal << " deleted. ";
//         std::cout << "[";
//         int pos = barWidth * progress;
//         for (int i = 0; i < barWidth; ++i) {
//             if (i < pos) std::cout << "|";
//             else if (i == pos) std::cout << "_"; //next percentage block; fill with cursor code after demonstration to Ronnie
//             else std::cout << " ";
//         }
//         std::cout << "] " << int(progress * 100.0) << " %\r";
//         std::cout.flush();

//         std::this_thread::sleep_for(std::chrono::milliseconds(1000));
//     }
//     std::cout << std::endl; 
// }

// // Function to delete a folder in a separate thread
// void deleteFolder(const fs::path& folderPath) {
//     try {
//         // Using the RD (Remove Directory) command to delete the folder
//         std::string command = "RD /S /Q \"" + folderPath.string() + "\""; 
//         deleteQueue.push_back(folderPath.string()); 
//         int result = system(command.c_str()); // Execute the command
//         if (result == 0) {
//             numUsersDeleted++; 
//         } else {
//             //std::cerr << "Failed to delete " << folderPath << " with error code: " << result << std::endl;
//         }
//         deleteQueue.erase(std::remove(deleteQueue.begin(), deleteQueue.end(), folderPath.string()), deleteQueue.end());
//     } catch (const std::exception& e) {
//         //std::cerr << "Failed to delete " << folderPath << ": " << e.what() << std::endl;
//     }
// }

// void removeAppData(fs::path& folderPath) {
//         fs::path appDataPath = folderPath / "AppData";
//         if (fs::exists(appDataPath)) { // Deletes AppData folder if one is found for that user
//             // Launch deleteFolder in a new thread and detach it
//             std::thread deleteThread(deleteFolder, appDataPath);
//             deleteThread.detach(); // Detach the thread so it runs independently
//         } else { // AppData folder not found 
//             noAppData++; 
//         }
//     }

// int mainLoop(){
//     std::string input; 
//     std::cout << "Enter the path of the Users folder to be cleaned and press Enter (or type 'default' to use 'C:/Users/' as the path):" << std::endl;
//     std::cin >> directoryPath;
//     std::cout << "Enter the name(s) [not the path] of User(s) folders to be kept. Type 'done' when you've entered every folder name to keep:" << std::endl; 
    
//     //populates keepUsers with inputed Users' folder names; breaks when 'done' is detected
//     while(input != "done"){ 
//         std::cin >> input; 
//         if (input == "done"){
//             break; 
//         }
//         keepUsers.push_back(input); 
//     }

//     for (int i = 0; i < directoryPath.length(); i++) {
//         directoryPath[i] = tolower(directoryPath[i]);
//     } 

//     if (directoryPath == "default") {
//         directoryPath = defaultUserPath; 
//     }

//     try {
//         for (auto& entry : fs::directory_iterator(directoryPath)) {
//             fs::path p = entry.path(); 
//             std::string userName = p.filename().string();

//             if (fs::is_directory(p) && std::find(std::begin(keepUsers), std::end(keepUsers), userName) == std::end(keepUsers)) {
//                 removeAppData(p); 
//             } else {
//                 std::cout << "Kept: " << userName << std::endl; 
//                 numUsersKept++; 
//             }
//         }
//     } catch (const fs::filesystem_error& e) {
//         std::cerr << "Filesystem error: " << e.what() << std::endl;
//     } catch (const std::exception& e) {
//         std::cerr << "General exception: " << e.what() << std::endl;
//     } 
//     std::cout << std::endl; 
//     return 0;
// }

// int main() {
//     mainLoop(); 
//     std::cout << "Number of Users kept: " << numUsersKept << std::endl;
//     std::cout << "Number of Users without an AppData folder: " << noAppData << std::endl; 
//     std::cout << "Deleting the rest of the Users' AppData folders. Keep this window open until that process completes. (some access errors may be thrown; these are normal and can be ignored)" << std::endl; 
//     std::cout << std::endl; 

//     if (deleteQueue.size() < 1){
//         std::cout << "No AppData folders to be deleted. Please check folder path and list of users to keep and try again." << std::endl; 
//     }
//     else{
//         progressBar(deleteQueue.size()); 
//         info = fs::space(fs::path(defaultUserPath).root_path()); // Re-check available space
//         // std::cout << "Freed up " << (startingSpace - info.available / bytesToMB)  << "MBs in " << 
//         std::cout << "Completed in: " << int(timeElapsed) / 60 << ":" << int(timeElapsed) % 60 << std::endl;
//     }
//     std::cout << "You may close the window or wait for it to close automatically" << std::endl; 
//     std::this_thread::sleep_for(std::chrono::milliseconds(15000));
//     return 0; 
// }
#include <iostream>
#include <filesystem>
#include <vector>
#include <algorithm>
#include <string>
#include <chrono>
#include <thread>
#include <windows.h>
#include <mutex>

namespace fs = std::filesystem;

// "keepUsers" are the directories that are skipped over
std::vector<std::string> keepUsers = {"colto", "clane", "sday2", "bwhittenbarger", "Administrator", "templocal", "Default", "Public", "Default User", "All Users", "astambaugh", "romay", "aboggs3", "jwhitt2", "jsturm"};
std::vector<std::string> deleteQueue = {}; // Vector of users' AppData folders being deleted
std::mutex queueMutex;                    // Mutex to synchronize queue operations

std::string defaultUserPath = "C:/Users/";
std::string directoryPath; // Updates to the user's entered path in mainLoop

double startingSpace;                     // Keeps the amount of space the system starts with (in MB)
std::filesystem::space_info info;         // Space_info variable to track disk space
double timeElapsed;
size_t totalDeletedBytes = 0;             // Tracks the total amount of data deleted (in bytes)

int bytesToMB = 1000000.0;
int numUsersKept = 0;
int numUsersDeleted = 0;
int noAppData = 0;

// Function to calculate the size of a directory
size_t calculateFolderSize(const fs::path& folderPath) {
    size_t totalSize = 0;
    try {
        for (const auto& entry : fs::recursive_directory_iterator(folderPath)) {
            if (fs::is_regular_file(entry.path())) {
                totalSize += fs::file_size(entry.path());
            }
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error calculating size of " << folderPath << ": " << e.what() << std::endl;
    }
    return totalSize;
}

// Function to display a progress bar
void progressBar(const int initialTotal, size_t totalDataSize) {
    int deletedCount = 0;
    size_t deletedData = 0;
    std::chrono::high_resolution_clock::time_point startTime = std::chrono::high_resolution_clock::now();

    while (deleteQueue.size() > 0 || deletedCount < initialTotal) {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            deletedCount = initialTotal - deleteQueue.size();
            deletedData = totalDeletedBytes;
        }
        float progress = static_cast<float>(deletedData) / totalDataSize;
        int barWidth = 70;

        std::chrono::duration<double> elapsed = std::chrono::high_resolution_clock::now() - startTime;
        timeElapsed = elapsed.count();

        std::cout << "(" << int(timeElapsed) / 60 << ":" << int(timeElapsed) % 60 << ") ";
        std::cout << deletedCount << " out of " << initialTotal << " deleted. ";
        std::cout << "Freed: " << deletedData / bytesToMB << " MB. ";
        std::cout << "[";

        int pos = barWidth * progress;
        for (int i = 0; i < barWidth; ++i) {
            if (i < pos) std::cout << "|";
            else if (i == pos) std::cout << "_";
            else std::cout << " ";
        }
        std::cout << "] " << int(progress * 100.0) << " %\r";
        std::cout.flush();

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    std::cout << std::endl;
}

// Function to delete a folder in a separate thread
void deleteFolder(const fs::path& folderPath, size_t folderSize) {
    try {
        {
            std::lock_guard<std::mutex> lock(queueMutex);
            deleteQueue.push_back(folderPath.string());
        }

        // Using the RD (Remove Directory) command to delete the folder
        std::string command = "RD /S /Q \"" + folderPath.string() + "\"";
        int result = system(command.c_str());
        if (result == 0) {
            numUsersDeleted++;
            {
                std::lock_guard<std::mutex> lock(queueMutex);
                totalDeletedBytes += folderSize;
            }
        }
        else{
            std::lock_guard<std::mutex> lock(queueMutex);
            deleteQueue.erase(std::remove(deleteQueue.begin(), deleteQueue.end(), folderPath.string()), deleteQueue.end());
        }
    } catch (const std::exception& e) {
        std::cerr << "Exception while deleting " << folderPath << ": " << e.what() << std::endl;
    }
}

// Function to process AppData
void removeAppData(fs::path& folderPath) {
    fs::path appDataPath = folderPath / "AppData";
    if (fs::exists(appDataPath)) {
        size_t folderSize = calculateFolderSize(appDataPath);
        std::thread deleteThread(deleteFolder, appDataPath, folderSize);
        deleteThread.detach();
    } else {
        noAppData++;
    }
}

// Main loop
int mainLoop() {
    std::string input;
    std::cout << "Enter the path of the Users folder to be cleaned and press Enter (or type 'default' to use 'C:/Users/' as the path):" << std::endl;
    std::cin >> directoryPath;

    std::cout << "Enter the name(s) [not the path] of User(s) folders to be kept. Type 'done' when you've entered every folder name to keep:" << std::endl;

    while (input != "done") {
        std::cin >> input;
        if (input == "done") {
            break;
        }
        keepUsers.push_back(input);
    }

    if (directoryPath == "default") {
        directoryPath = defaultUserPath;
    }

    try {
        for (const auto& entry : fs::directory_iterator(directoryPath)) {
            fs::path p = entry.path();
            std::string userName = p.filename().string();

            if (fs::is_directory(p) && std::find(std::begin(keepUsers), std::end(keepUsers), userName) == std::end(keepUsers)) {
                removeAppData(p);
            } else {
                std::cout << "Kept: " << userName << std::endl;
                numUsersKept++;
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}

int main() {
    mainLoop();
    size_t totalDataSize = totalDeletedBytes;

    if (deleteQueue.size() < 1) {
        std::cout << "No AppData folders to delete. Please check folder path and list of users to keep." << std::endl;
    } else {
        progressBar(deleteQueue.size(), totalDataSize);
        info = fs::space(fs::path(defaultUserPath).root_path());
        std::cout << "Completed in: " << int(timeElapsed) / 60 << ":" << int(timeElapsed) % 60 << std::endl;
        std::cout << "Total freed: " << totalDeletedBytes / bytesToMB << " MB" << std::endl;
    }

    return 0;
}
