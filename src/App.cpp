#include "../include/App/App.hpp"
#include "../include/App/SearchFrame.hpp"




static void loadConfig(State& state) {
}


wxIMPLEMENT_APP(App);

bool App::OnInit() {
    loadConfig(_state);
    SearchFrame*  frame =  new SearchFrame(&_state,"hello");
    frame->Show();
    return true;
} 
