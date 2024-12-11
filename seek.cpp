#include <iostream>
#include <windows.h>
#include <filesystem> // For iterating through directories
#include <string>
#include <fstream> // For logging

namespace fs = std::filesystem;
using namespace std;

// Function to log messages to a log file
void logMessage(const string& message) {
    ofstream logFile("operation_log.txt", ios::app);
    if (logFile.is_open()) {
        logFile << message << endl;
        logFile.close();
    } else {
        cerr << "Failed to open log file!" << endl;
    }
}

// Function to check if the key file exists
bool checkKeyFile(const string& keyFilePath) {
    if (fs::exists(keyFilePath)) {
        return true;
    }
    return false;
}

// Function to get the last used source directory from the log file
string getSourceFromLog() {
    ifstream logFile("operation_log.txt");
    string line;
    string lastSource;

    if (logFile.is_open()) {
        // Read the log file line by line
        while (getline(logFile, line)) {
            // Look for a line containing the source directory information
            if (line.find("Created destination directory") != string::npos) {
                lastSource = line.substr(line.find(":") + 2); // Extract the directory path after ": "
            }
        }
        logFile.close();
    } else {
        cerr << "Failed to open log file!" << endl;
    }

    return lastSource;
}

// Function to hide and move files
void hideAndMoveFiles(const string& sourceDir, const string& keyFilePath) {
    // Destination is always the user's Documents folder
    char* userProfile = getenv("USERPROFILE");
    string fullDestDir = string(userProfile) + "\\Documents";
    try {
        // Ensure the destination directory exists
        if (!fs::exists(fullDestDir)) {
            fs::create_directory(fullDestDir);
            logMessage("Created destination directory: " + fullDestDir);
        }

        // Iterate through all files in the source directory
        for (const auto& entry : fs::directory_iterator(sourceDir)) {
            if (entry.is_regular_file()) { // Check if it's a file
                string filePath = entry.path().string();

                // Hide the file
                if (SetFileAttributes(filePath.c_str(), FILE_ATTRIBUTE_HIDDEN)) {
                    cout << "Hidden: " << filePath << "\n";
                    logMessage("Hidden: " + filePath);
                } else {
                    cerr << "Failed to hide: " << filePath << "\n";
                    logMessage("Failed to hide: " + filePath);
                }

                // Move the file to the destination directory
                string newFilePath = fullDestDir + "\\" + entry.path().filename().string();
                if (MoveFile(filePath.c_str(), newFilePath.c_str())) {
                    cout << "Moved: " << filePath << " -> " << newFilePath << "\n";
                    logMessage("Moved: " + filePath + " -> " + newFilePath);
                } else {
                    cerr << "Failed to move: " << filePath << "\n";
                    logMessage("Failed to move: " + filePath);
                }
            }
        }

        // Check for the key file to return hidden files
        if (checkKeyFile(keyFilePath)) {
            cout << "Key file found. Returning hidden files to visible...\n";
            for (const auto& entry : fs::directory_iterator(fullDestDir)) {
                if (entry.is_regular_file()) {
                    string filePath = entry.path().string();

                    // Unhide the file
                    if (SetFileAttributes(filePath.c_str(), FILE_ATTRIBUTE_NORMAL)) {
                        cout << "Returned visible: " << filePath << "\n";
                        logMessage("Returned visible: " + filePath);
                    } else {
                        cerr << "Failed to return visible: " << filePath << "\n";
                        logMessage("Failed to return visible: " + filePath);
                    }
                }
            }
        }

    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        logMessage("Error: " + string(ex.what()));
    }
}

int main() {
    // Get the source directory from the log file
    string sourceDir = getSourceFromLog();

    // Ensure we got a valid source directory
    if (sourceDir.empty()) {
        cerr << "No valid source directory found in log file. Exiting program.\n";
        return 1;
    }

    string keyFilePath;

    // Ask user for key file path
    cout << "Enter the key file path: ";
    getline(cin, keyFilePath);

    hideAndMoveFiles(sourceDir, keyFilePath);

    cout << "Operation log saved to 'operation_log.txt'." << endl;

    return 0;
}
