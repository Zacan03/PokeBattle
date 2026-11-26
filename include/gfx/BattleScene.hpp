#pragma once
#include <SFML/Graphics.hpp>
#include <string>
#include "game/Battle.hpp"
#include "gfx/UI.hpp"
using namespace std;

class BattleScene {
public:
    BattleScene();
    bool init();
    void run();

private:
    enum class Phase { Menu, Playing, GameOver };
    enum class Mode { Command, Moves };

    struct SpriteEffect {
        float lunge, flash, alpha;
        bool fainted;
        SpriteEffect() : lunge(0), flash(0), alpha(1), fainted(false) {}
    };
    struct Projectile {
        bool active; float progress, duration; sf::Vector2f from, to; sf::Color color; int kind;
        Projectile() : active(false), progress(0), duration(0.45f), from(), to(), color(), kind(0) {}
    };

    void handleevent(const sf::Event& event);
    void update(float seconds);
    void render();
    void startbattle();
    void resetvisuals();
    void processnextevent();
    void activatecommand(int index);
    void confirmmove(int index);
    void startundo();
    void startreplay();
    void newbattle();
    void goback();
    void finishplayback();
    int hittest(sf::Vector2f logical) const;

    sf::Vector2f spritecenter(bool isplayerone) const;
    void buildbackground();
    void drawpokemon(bool isplayerone);
    void drawprojectile();
    void drawhud();
    void drawhudbox(float x, float y, float width, float height, bool tailleft);
    void drawhealthbar(float x, float y, const ui::HealthBar& bar, StatusType condition);
    void drawbottom();
    void drawdialogframe();
    void drawmenuframe(float x, float width);
    void drawcursor(float x, float y);
    void rect(float x, float y, float width, float height, sf::Color color);
    void text(const string& line, float x, float baseline, unsigned size, sf::Color color,
              sf::Color shadow, bool shadowon = true);
    void outlined(const string& line, float x, float baseline, unsigned size, sf::Color color, sf::Color outline);
    float textwidth(const string& line, unsigned size);
    int wrap(const string& line, float maxwidth, unsigned size, string* lines, int maxlines);
    string shout(string line) const;

    sf::RenderWindow window;
    sf::Font font;
    Battle battle;
    Phase phase;
    Mode mode;
    int commandchoice, movechoice;

    sf::Texture textures[2];
    bool hastexture[2];
    sf::Sprite sprites[2];
    sf::Texture backgroundtexture;
    sf::Sprite backgroundsprite;
    SpriteEffect effects[2];
    Projectile projectile;

    ui::HealthBar healthbars[2];
    StatusType shownstatus[2];
    string message;
    float eventtimer;
    float quake;
    float elapsed;
};
