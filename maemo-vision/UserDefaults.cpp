#include "UserDefaults.h"

#include <fstream>
#include <FCam/TagValue.h>
#include <iostream>
#include <QDir>

UserDefaults * UserDefaults::_instance = NULL;

UserDefaults::UserDefaults(QString path) : filename(path) {
    std::ifstream file(path.toAscii().data());
    FCam::TagValue value;
    std::string key;
    while (file.good()) {
        file >> std::ws >> key >> std::ws >> value >> std::ws;
        (*this)[QString(key.c_str())] = value;
    } 
    file.close();
}

void UserDefaults::commit(){
    std::cout << "Committing user settings\n";
    std::ofstream file(filename.toAscii().data());
    foreach (QString key, keys()) {
        std::cout << key.toStdString() << " = " << (*this)[key] << "\n";    
        file << key.toStdString() << " " << (*this)[key] << "\n";    
    }
    file.close();
    std::cout << "Done committing\n";
}

UserDefaults &UserDefaults::instance() {
    if (_instance == NULL) {
        QDir dir;
        dir.mkpath("/home/user/.fcamera");
        _instance = new UserDefaults("/home/user/.fcamera/settings");
    }
    return *_instance;
    
}

