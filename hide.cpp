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

void hideAndMoveFiles(const string& sourceDir, const string& destDir) {
    string fullDestDir = "C:\\" + destDir;
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
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        logMessage("Error: " + string(ex.what()));
    }
}

int main() {
    // Source directory remains as the Documents folder
    char* userProfile = getenv("USERPROFILE");
    string sourceDir = string(userProfile) + "\\Documents";
    string destDir;

    //cout << "Source directory (Documents folder): " << sourceDir << endl;
    cout << "Enter the destination directory: ";
    getline(cin, destDir);

    hideAndMoveFiles(sourceDir, destDir);

    cout << "Operation log saved to 'operation_log.txt'." << endl;

    return 0;
}
