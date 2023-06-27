//#ifndef GLOBAL_H
//#define GLOBAL_H

////#include "parsers/showparser.h"

////#include "parsers/providers/gogoanime.h"
////#include "parsers/providers/nivod.h"
////#include "parsers/providers/ntdongman.h"
////#include "parsers/providers/huale.h"

////#include <parsers/providers/nineanimehq.h>

//class Global : public QObject {

//    Q_OBJECT
////    Q_PROPERTY(ShowResponseObject* currentShowObject READ currentShowObject CONSTANT)
////    Q_PROPERTY(QList<ShowParser*> providers READ providers CONSTANT)
////    Q_PROPERTY(ShowParser* currentSearchProvider READ getCurrentSearchProvider NOTIFY currentSearchProviderChanged)


////    ShowParser* m_currentSearchProvider;
////    ShowResponseObject m_currentShowObject{this};
//public:
////    inline ShowResponseObject* currentShowObject() {
////        return &m_currentShowObject;
////    };
////    inline ShowParser* getProvider(int provider) const {
////        if(providersMap.contains(provider)) return providersMap[provider];
////        return nullptr;
////    }
////    inline ShowParser* getCurrentShowProvider() {
////        return providersMap[m_currentShowObject.provider()];
////    }
////    inline ShowParser* getCurrentSearchProvider() {
////        Q_ASSERT(m_currentSearchProvider!=nullptr);
////        return m_currentSearchProvider;
////    }
////    inline QList<ShowParser*> providers() const {
////        return providersMap.values ();
////    }
////    inline Q_INVOKABLE void changeSearchProvider(int providerEnum) {
////        m_currentSearchProvider = providersMap[providerEnum];
////        emit currentSearchProviderChanged();
////    }


//    static Global& instance() {
//        static Global instance;
//        return instance;
//    }
//private:
//    Global() {
////        m_currentSearchProvider = providersMap[Providers::e_Gogoanime];
//    }
//    ~Global(){
////        m_currentSearchProvider=nullptr;
////        for (auto it = providersMap.begin(); it != providersMap.end(); ++it) {
////            delete it.value();
////        }
////        providersMap.clear ();
//    }
//    Global(const Global&) = delete; // delete copy constructor
//    Global& operator=(const Global&) = delete; // delete copy assignment operator
//signals:
//    void currentSearchProviderChanged(void);
//};
//#endif // GLOBAL_H
