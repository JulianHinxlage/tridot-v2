//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#ifndef TRIDOT_SIGNAL_H
#define TRIDOT_SIGNAL_H

#include <functional>

namespace ecs {

    template<typename... Args>
    class SignalRef;

    template<typename... Args>
    class Signal {
    public:
        typedef std::function<void(Args...)> Callback;

        void add(const Callback &callback, const std::string &name = ""){
            listeners.emplace_back(callback, name);
        }

        void add(const std::string &name, const Callback &callback){
            listeners.emplace_back(callback, name);
        }

        void remove(const std::string &name){
            for(int i = 0; i < listeners.size(); i++){
                auto &listener = listeners[i];
                if(listener.name == name){
                    listeners.erase(listeners.begin() + i);
                    i--;
                }
            }
        }

        void invoke(Args... args){
            for(auto &listener : listeners){
                if(listener.callback != nullptr){
                    listener.callback(args...);
                }
            }
        }

        bool order(const std::vector<std::string> &names){
            for(int i = 0; i < names.size(); i++){
                if(i != 0) {
                    for (auto &listener : listeners) {
                        if (listener.name == names[i]) {
                            listener.dependencies.push_back(names[i - 1]);
                        }
                    }
                }
            }
            return sort();
        }

        SignalRef<Args...> ref(){
            return SignalRef<Args...>(this);
        }

    private:
        class Listener{
        public:
            Callback callback;
            std::string name;
            std::vector<std::string> dependencies;

            Listener(const Callback &callback = nullptr, const std::string &name = "")
                : callback(callback), name(name){}
        };
        std::vector<Listener> listeners;

        bool sort(){
            bool conflict = false;
            std::vector<std::string> swaps;
            for (int i = 0; i < listeners.size(); i++) {
                int start = i;
                for (auto &dependency : listeners[i].dependencies) {
                    for (int j = i+1; j < listeners.size(); j++) {
                        if (listeners[j].name == dependency) {

                            bool loop = false;
                            for(auto &s : swaps){
                                if(s == listeners[j].name){
                                    loop = true;
                                    conflict = true;
                                    break;
                                }
                            }

                            if(!loop) {
                                listeners.insert(listeners.begin() + i, listeners[j]);
                                listeners.erase(listeners.begin() + j + 1);
                                i++;
                            }
                        }
                    }
                }
                if(i != start){
                    swaps.push_back(listeners[i].name);
                    i = start - 1;
                }else{
                    swaps.clear();
                }
            }
            return conflict;
        }
    };

    template<typename... Args>
    class SignalRef {
    public:
        typedef typename Signal<Args...>::Callback Callback;

        SignalRef(Signal<Args...> *signal) : signal(signal) {}

        void add(const Callback &callback, const std::string &name = ""){
            signal->add(callback, name);
        }

        void add(const std::string &name, const Callback &callback){
            signal->add(name, callback);
        }

        void remove(const std::string &name){
            signal->remove(name);
        }

        bool order(const std::vector<std::string> &names){
            return signal->order(names);
        }

    private:
        Signal<Args...> *signal;
    };

}

#endif //TRIDOT_SIGNAL_H