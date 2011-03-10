#ifndef USER_DEFAULTS
#define USER_DEFAULTS


#include <QHash>
#include <FCam/TagValue.h>
#include <FCam/Time.h>

#include <string>
#include <vector>

/** The UserDefaults class maintains the persistent settings (stored
 * in ~/.fcamera/settings). Access keys and values as you would in a
 * QHash. After writing your settings call commit to sync to disk.
 *
 * Setting a value:
 * UserDefaults::instance()["mySetting"] = 5;
 * UserDefaults::instance().commit();
 *
 * Checking if a value exists:
 * if (UserDefaults::instance()["mySetting"].valid()) ...
 *
 * Getting a value:
 * int mySetting = UserDefaults::instance()["mySetting"];
 *
 * You can save any type that an FCam::TagValue can take on, which
 * includes ints, floats, doubles, std::strings, FCam::Times, and
 * std::vectors of the above.
 */
class UserDefaults : public QHash<QString, FCam::TagValue> {
public:
    // Save all changes to keys/values to disk
    void commit();                
    
    // Get a handle to the singleton instance of UserDefaults
    static UserDefaults &instance();

private:
    // It's a singleton, so the constructor is private, and copying
    // and assignment are disallowed.
    UserDefaults(QString filename);   
    UserDefaults(const UserDefaults &) : QHash<QString, FCam::TagValue>() {}
    UserDefaults &operator=(const UserDefaults &) {return *this;}

    // The sole instance of this class
    static UserDefaults *_instance; 

    // The file where the settings are stored
    QString filename;

    // The settings
    QHash<QString, FCam::TagValue> data;


};


#endif
