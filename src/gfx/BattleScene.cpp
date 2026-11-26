#include "gfx/BattleScene.hpp"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <ctime>
using namespace std;

namespace {
const float scale = 4.f;
const float windowwidth = 240.f * scale, windowheight = 160.f * scale;
const float lungetime = 0.35f, flashtime = 0.40f;
const float pi = 3.14159265f;

const unsigned bigsize = 14, hudsize = 12, smallsize = 9;

const char* fontpaths[] = {
    "assets/fonts/font.ttf",
    "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf", "/usr/share/fonts/TTF/DejaVuSans.ttf",
    "C:/Windows/Fonts/segoeui.ttf", "C:/Windows/Fonts/arial.ttf",
    "/System/Library/Fonts/Supplemental/Arial.ttf", "/Library/Fonts/Arial.ttf"
};

sf::Color shade(unsigned value) { return ui::fromhex(value); }

enum EffectKind { BallEffect = 0, FlameEffect, ClawEffect, SlashEffect, QuakeEffect, LeafEffect, PulseEffect, StreakEffect, ThunderEffect, EffectTotal };
const float effectduration[EffectTotal] = { 0.45f, 0.85f, 0.75f, 0.55f, 0.85f, 0.75f, 0.65f, 0.40f, 0.75f };

int effectfor(const string& movename) {
    if (movename == "Flamethrower")   return FlameEffect;
    if (movename == "Dragon Claw")    return ClawEffect;
    if (movename == "Air Slash")      return SlashEffect;
    if (movename == "Earthquake")     return QuakeEffect;
    if (movename == "Leaf Blade")     return LeafEffect;
    if (movename == "Dragon Pulse")   return PulseEffect;
    if (movename == "Quick Attack")   return StreakEffect;
    if (movename == "Thunder Punch")  return ThunderEffect;
    return BallEffect;
}
float noise(int first, int second) { float value = sin(first * 12.9898f + second * 78.233f) * 43758.5453f; return value - floor(value); }
sf::Vector2f blend(sf::Vector2f start, sf::Vector2f finish, float amount) { return start + (finish - start) * amount; }
void drawline(sf::RenderTarget& canvas, sf::Vector2f start, sf::Vector2f finish, float thickness, sf::Color color) {
    sf::Vector2f delta = finish - start;
    float length = sqrt(delta.x * delta.x + delta.y * delta.y);
    if (length < 0.5f) return;
    sf::RectangleShape bar(sf::Vector2f(length, thickness));
    bar.setOrigin(0.f, thickness / 2.f); bar.setPosition(start); bar.setRotation(atan2(delta.y, delta.x) * 180.f / pi);
    bar.setFillColor(color);
    canvas.draw(bar);
}
void drawdot(sf::RenderTarget& canvas, sf::Vector2f center, float radius, sf::Color color) {
    sf::CircleShape circle(radius); circle.setOrigin(radius, radius); circle.setPosition(center); circle.setFillColor(color); canvas.draw(circle);
}
uint32_t newseed() { return (uint32_t)time(nullptr) * 2654435761u + 12345u; }

string uppercase(string line) {
    for (size_t i = 0; i < line.size(); ++i) if (line[i] >= 'a' && line[i] <= 'z') line[i] = (char)(line[i] - 32);
    return line;
}

const char* commandlabels[4] = { "FIGHT", "UNDO", "REPLAY", "NEW" };
const float commandx[4] = { 136.f, 192.f, 136.f, 192.f };
const float rowbaseline[2] = { 133.f, 148.f };
const float movex[2] = { 16.f, 88.f };
}

BattleScene::BattleScene()
    : window(sf::VideoMode((unsigned)windowwidth, (unsigned)windowheight), "Charizard vs Sceptile - DSA Battle Simulator",
             sf::Style::Titlebar | sf::Style::Close),
      font(), battle(newseed()), phase(Phase::Menu), mode(Mode::Command), commandchoice(0), movechoice(0),
      textures(), sprites(), backgroundtexture(), backgroundsprite(), effects(), projectile(), healthbars(), message(),
      eventtimer(0.f), quake(0.f), elapsed(0.f) {
    hastexture[0] = hastexture[1] = false;
    shownstatus[0] = shownstatus[1] = StatusType::None;
    window.setFramerateLimit(60);
}

bool BattleScene::init() {
    bool fontloaded = false;
    for (size_t i = 0; i < sizeof(fontpaths) / sizeof(fontpaths[0]) && !fontloaded; ++i)
        fontloaded = font.loadFromFile(fontpaths[i]);
    if (!fontloaded) { fprintf(stderr, "No font found. Put a .ttf at assets/fonts/font.ttf\n"); return false; }
    font.setSmooth(false);

    const char* spritepaths[2] = { "assets/sprites/charizard.png", "assets/sprites/sceptile.png" };
    for (int i = 0; i < 2; ++i) {
        if (textures[i].loadFromFile(spritepaths[i])) {
            textures[i].setSmooth(false);
            hastexture[i] = true;
            sprites[i].setTexture(textures[i], true);
            sprites[i].setScale(scale, scale);
        }
    }
    buildbackground();
    resetvisuals();
    startbattle();
    return true;
}

void BattleScene::buildbackground() {
    sf::Image image;
    image.create(240, 160, shade(0xF0F0F0));
    for (unsigned y = 0; y < 160; ++y) {
        sf::Color linecolor;
        if (y < 12)       linecolor = (y % 4 == 0) ? shade(0xF0F8F8) : (y < 4 ? shade(0xF8F8B0) : y < 8 ? shade(0xF8F8C0) : shade(0xF8F8D8));
        else if (y < 30)  linecolor = (y == 13 || y == 14 || y == 15 || y == 17 || y == 19) ? shade(0xF8F8E8) : shade(0xF8F8F8);
        else if (y == 111) linecolor = shade(0xD0D0D0);
        else              linecolor = (y % 4 == 0) ? shade(0xF0F8F8) : shade(0xF0F0F0);
        for (unsigned x = 0; x < 240; ++x) image.setPixel(x, y, linecolor);
    }
    struct Ellipse { float centerx, centery, radiusx, radiusy; };
    const Ellipse platforms[2] = { { 176.f, 61.f, 64.f, 12.f }, { 40.f, 118.f, 90.f, 16.f } };
    for (int platform = 0; platform < 2; ++platform) {
        for (int y = 0; y < 111; ++y) for (int x = 0; x < 240; ++x) {
            float offsetx = (x + 0.5f - platforms[platform].centerx) / platforms[platform].radiusx, offsety = (y + 0.5f - platforms[platform].centery) / platforms[platform].radiusy;
            float squared = offsetx * offsetx + offsety * offsety;
            if (squared <= 1.f) image.setPixel(x, y, squared > 0.82f ? shade(0xE0E0E0) : shade(0xF8F8F8));
        }
    }
    backgroundtexture.loadFromImage(image);
    backgroundtexture.setSmooth(false);
    backgroundsprite.setTexture(backgroundtexture, true);
    backgroundsprite.setScale(scale, scale);
}

void BattleScene::startbattle() {
    battle.begin();
    message = shout("A battle begins: " + battle.playerone().name + " vs " + battle.playertwo().name + "!");
    phase = Phase::Playing;
    eventtimer = 1.6f;
}

void BattleScene::resetvisuals() {
    healthbars[0].snap(battle.playerone().healthfraction());
    healthbars[1].snap(battle.playertwo().healthfraction());
    shownstatus[0] = shownstatus[1] = StatusType::None;
    for (int i = 0; i < 2; ++i) effects[i] = SpriteEffect();
    projectile.active = false;
    message.clear();
    mode = Mode::Command; commandchoice = 0; movechoice = 0;
    phase = Phase::Menu;
}

void BattleScene::confirmmove(int index) {
    if (phase != Phase::Menu) return;
    if (battle.submitmove(index)) {
        phase = Phase::Playing;
        eventtimer = 0.f;
        mode = Mode::Command; commandchoice = 0; movechoice = 0;
    }
}

void BattleScene::activatecommand(int index) {
    if (phase == Phase::Playing) return;
    switch (index) {
        case 0: if (battle.awaitinginput()) { mode = Mode::Moves; movechoice = 0; } break;
        case 1: startundo(); break;
        case 2: startreplay(); break;
        case 3: newbattle(); break;
    }
}

void BattleScene::startundo() {
    if (phase == Phase::Playing) return;
    if (battle.undolastturn()) { phase = Phase::Playing; eventtimer = 0.f; mode = Mode::Command; commandchoice = 0; }
    else message = "Nothing to undo.";
}

void BattleScene::startreplay() {
    if (phase == Phase::Playing) return;
    if (battle.replaylastturn()) { phase = Phase::Playing; eventtimer = 0.f; mode = Mode::Command; commandchoice = 0; }
    else message = "Nothing to replay.";
}

void BattleScene::newbattle() {
    battle.reset(newseed());
    resetvisuals();
    startbattle();
}

void BattleScene::goback() { if (mode == Mode::Moves) mode = Mode::Command; }

int BattleScene::hittest(sf::Vector2f position) const {
    if (phase == Phase::Playing) return -1;
    for (int i = 0; i < 4; ++i) {
        float x = (mode == Mode::Command) ? commandx[i] : movex[i % 2];
        float baseline = rowbaseline[i / 2];
        float width = (mode == Mode::Command) ? 52.f : 70.f;
        if (position.x >= x - 8.f && position.x <= x - 8.f + width + 8.f && position.y >= baseline - 11.f && position.y <= baseline + 3.f) return i;
    }
    return -1;
}

void BattleScene::handleevent(const sf::Event& event) {
    if (event.type == sf::Event::Closed) { window.close(); return; }

    if (event.type == sf::Event::MouseMoved) {
        sf::Vector2f position = window.mapPixelToCoords(sf::Vector2i(event.mouseMove.x, event.mouseMove.y)) / scale;
        int hit = hittest(position);
        if (hit >= 0) { if (mode == Mode::Command) commandchoice = hit; else movechoice = hit; }
    }
    if (event.type == sf::Event::MouseButtonPressed) {
        if (phase == Phase::Playing) { eventtimer = 0.f; return; }
        if (event.mouseButton.button == sf::Mouse::Right) { goback(); return; }
        if (event.mouseButton.button == sf::Mouse::Left) {
            sf::Vector2f position = window.mapPixelToCoords(sf::Vector2i(event.mouseButton.x, event.mouseButton.y)) / scale;
            int hit = hittest(position);
            if (hit >= 0) { if (mode == Mode::Command) { commandchoice = hit; activatecommand(hit); } else { movechoice = hit; confirmmove(hit); } }
        }
    }
    if (event.type == sf::Event::KeyPressed) {
        const sf::Keyboard::Key key = event.key.code;
        if (phase == Phase::Playing) {
            if (key == sf::Keyboard::Return || key == sf::Keyboard::Space || key == sf::Keyboard::Z) eventtimer = 0.f;
            if (key == sf::Keyboard::Escape) window.close();
            return;
        }
        int& selection = (mode == Mode::Command) ? commandchoice : movechoice;
        int column = selection % 2, row = selection / 2;
        if (key == sf::Keyboard::Left  || key == sf::Keyboard::A) column = 0;
        if (key == sf::Keyboard::Right || key == sf::Keyboard::D) column = 1;
        if (key == sf::Keyboard::Up    || key == sf::Keyboard::W) row = 0;
        if (key == sf::Keyboard::Down  || key == sf::Keyboard::S) row = 1;
        selection = row * 2 + column;
        if (key == sf::Keyboard::Return || key == sf::Keyboard::Space || key == sf::Keyboard::Z) {
            if (mode == Mode::Command) activatecommand(commandchoice); else confirmmove(movechoice);
        }
        if (mode == Mode::Moves && key >= sf::Keyboard::Num1 && key <= sf::Keyboard::Num4) confirmmove(key - sf::Keyboard::Num1);
        if (key == sf::Keyboard::BackSpace || key == sf::Keyboard::X) goback();
        if (key == sf::Keyboard::Escape) { if (mode == Mode::Moves) goback(); else window.close(); }
        if (key == sf::Keyboard::U) startundo();
        if (key == sf::Keyboard::R) startreplay();
        if (key == sf::Keyboard::N) newbattle();
    }
}

string BattleScene::shout(string line) const {
    string names[10];
    int count = 0;
    names[count++] = battle.playerone().name; names[count++] = battle.playertwo().name;
    for (int i = 0; i < 4; ++i) { names[count++] = battle.playerone().moves[i]; names[count++] = battle.playertwo().moves[i]; }
    for (int i = 0; i < count; ++i) {
        if (names[i].empty()) continue;
        string loud = uppercase(names[i]);
        size_t position = 0;
        while ((position = line.find(names[i], position)) != string::npos) { line.replace(position, names[i].size(), loud); position += loud.size(); }
    }
    return line;
}

void BattleScene::processnextevent() {
    BattleEvent event;
    if (!battle.events().dequeue(event)) return;

    healthbars[0].settarget((float)event.playeronehealth / (float)battle.playerone().maxhealth);
    healthbars[1].settarget((float)event.playertwohealth / (float)battle.playertwo().maxhealth);
    shownstatus[0] = event.playeronestatus;
    shownstatus[1] = event.playertwostatus;
    if (!event.text.empty()) message = shout(event.text);

    const int actorindex = event.actorisplayerone ? 0 : 1, targetindex = event.targetisplayerone ? 0 : 1;
    eventtimer = 0.9f;

    switch (event.type) {
        case EventType::Sync:
            effects[0].fainted = event.playeronehealth <= 0;
            effects[1].fainted = event.playertwohealth <= 0;
            eventtimer = 0.8f;
            break;
        case EventType::TurnStart:
            eventtimer = event.amount == 1 ? 0.1f : 0.05f;
            break;
        case EventType::MoveUsed:
            effects[actorindex].lunge = lungetime;
            projectile.kind = effectfor(event.movename);
            projectile.active = true; projectile.progress = 0.f; projectile.duration = effectduration[projectile.kind];
            projectile.from = spritecenter(event.actorisplayerone) + sf::Vector2f(0.f, -22.f);
            projectile.to = spritecenter(event.targetisplayerone) + sf::Vector2f(0.f, -22.f);
            projectile.color = ui::typecolor(event.movetype);
            if (projectile.kind == QuakeEffect) quake = projectile.duration;
            eventtimer = max(0.9f, projectile.duration + 0.25f);
            break;
        case EventType::Damage:
            effects[targetindex].flash = flashtime;
            eventtimer = event.text.empty() ? 0.9f : 1.3f;
            break;
        case EventType::StatusApplied: eventtimer = 1.2f; break;
        case EventType::StatusDamage:
            effects[targetindex].flash = 0.25f;
            break;
        case EventType::Faint:
            effects[targetindex].fainted = true;
            eventtimer = 1.4f;
            break;
        case EventType::BattleEnd: eventtimer = 0.9f; break;
        default: break;
    }
}

void BattleScene::finishplayback() {
    phase = battle.isover() ? Phase::GameOver : Phase::Menu;
    mode = Mode::Command;
    if (phase == Phase::GameOver && commandchoice == 0) commandchoice = 1;
}

void BattleScene::update(float seconds) {
    elapsed += seconds;
    healthbars[0].update(seconds); healthbars[1].update(seconds);
    for (int i = 0; i < 2; ++i) {
        effects[i].lunge = max(0.f, effects[i].lunge - seconds);
        effects[i].flash = max(0.f, effects[i].flash - seconds);
        effects[i].alpha = effects[i].fainted ? max(0.f, effects[i].alpha - seconds * 1.4f)
                                              : min(1.f, effects[i].alpha + seconds * 3.f);
    }
    if (projectile.active) { projectile.progress += seconds / projectile.duration; if (projectile.progress >= 1.f) projectile.active = false; }
    quake = max(0.f, quake - seconds);

    if (phase == Phase::Playing) {
        eventtimer -= seconds;
        if (eventtimer <= 0.f) {
            if (battle.events().empty()) finishplayback();
            else processnextevent();
        }
    }
}

void BattleScene::rect(float x, float y, float width, float height, sf::Color color) {
    sf::RectangleShape shape(sf::Vector2f(width * scale, height * scale));
    shape.setPosition(x * scale, y * scale);
    shape.setFillColor(color);
    window.draw(shape);
}

float BattleScene::textwidth(const string& line, unsigned size) {
    sf::Text label(line, font, size * (unsigned)scale);
    return label.getLocalBounds().width / scale;
}

void BattleScene::text(const string& line, float x, float baseline, unsigned size, sf::Color color,
                       sf::Color shadow, bool shadowon) {
    const unsigned pixels = size * (unsigned)scale;
    for (int pass = shadowon ? 0 : 1; pass < 2; ++pass) {
        sf::Text label(line, font, pixels);
        label.setFillColor(pass == 0 ? shadow : color);
        float offset = pass == 0 ? 1.f : 0.f;
        label.setPosition(floor((x + offset) * scale), floor((baseline + offset) * scale) - (float)pixels);   // SFML baseline = position.y + size
        window.draw(label);
    }
}

void BattleScene::outlined(const string& line, float x, float baseline, unsigned size, sf::Color color, sf::Color outline) {
    const float offsets[4][2] = { {-1, 0}, {1, 0}, {0, -1}, {0, 1} };
    for (int i = 0; i < 4; ++i) text(line, x + offsets[i][0], baseline + offsets[i][1], size, outline, outline, false);
    text(line, x, baseline, size, color, color, false);
}

int BattleScene::wrap(const string& line, float maxwidth, unsigned size, string* lines, int maxlines) {
    int linecount = 0;
    string current, word;
    for (size_t i = 0; i <= line.size(); ++i) {
        if (i == line.size() || line[i] == ' ') {
            if (!word.empty()) {
                string trial = current.empty() ? word : current + " " + word;
                if (!current.empty() && textwidth(trial, size) > maxwidth) {
                    if (linecount < maxlines) lines[linecount++] = current;
                    current = word;
                } else current = trial;
                word.clear();
            }
        } else word += line[i];
    }
    if (!current.empty() && linecount < maxlines) lines[linecount++] = current;
    return linecount;
}

sf::Vector2f BattleScene::spritecenter(bool isplayerone) const {
    return isplayerone ? sf::Vector2f(72.f * scale, 84.f * scale) : sf::Vector2f(176.f * scale, 40.f * scale);
}

void BattleScene::drawpokemon(bool isplayerone) {
    const int index = isplayerone ? 0 : 1;
    const SpriteEffect& effect = effects[index];
    if (effect.alpha <= 0.01f) return;

    sf::Vector2f position = isplayerone ? sf::Vector2f(40.f, 52.f) : sf::Vector2f(144.f, 8.f);
    if (effect.lunge > 0.f) {
        float progress = 1.f - effect.lunge / lungetime;
        float offset = sin(progress * pi) * 14.f;
        position.x += isplayerone ? offset : -offset;
    }
    if (effect.fainted) position.y += (1.f - effect.alpha) * 10.f;
    position.x = floor(position.x + 0.5f); position.y = floor(position.y + 0.5f);

    float alpha = 255.f * effect.alpha;
    if (effect.flash > 0.f && fmod(effect.flash * 18.f, 2.f) < 1.f) alpha *= 0.25f;

    if (hastexture[index]) {
        sprites[index].setPosition(position * scale);
        sprites[index].setColor(sf::Color(255, 255, 255, (sf::Uint8)alpha));
        window.draw(sprites[index]);
    } else {
        sf::CircleShape body(30.f * scale);
        body.setOrigin(30.f * scale, 30.f * scale); body.setPosition((position + sf::Vector2f(32.f, 32.f)) * scale);
        body.setFillColor(isplayerone ? sf::Color(240, 130, 50, (sf::Uint8)alpha) : sf::Color(90, 190, 100, (sf::Uint8)alpha));
        window.draw(body);
    }
}

void BattleScene::drawprojectile() {
    if (!projectile.active) return;
    const float progress = min(1.f, projectile.progress);
    const sf::Vector2f origin = projectile.from, target = projectile.to;
    sf::RenderTarget& canvas = window;

    switch (projectile.kind) {
    case FlameEffect:
        for (int i = 0; i < 20; ++i) {
            float along = progress * 1.25f - i * 0.035f;
            if (along <= 0.f || along > 1.f) continue;
            sf::Vector2f position = blend(origin, target, along);
            position.x += (noise(i, 1) - 0.5f) * 40.f * along;
            position.y += (noise(i, 2) - 0.5f) * 40.f * along - sin(along * pi) * 18.f;
            sf::Color flamecolor = along < 0.4f ? sf::Color(255, 235, 100) : along < 0.75f ? sf::Color(255, 140, 40) : sf::Color(220, 60, 30);
            flamecolor.a = (sf::Uint8)(235 * (1.f - 0.5f * along));
            drawdot(canvas, position, 8.f + 16.f * along, flamecolor);
        }
        break;
    case ClawEffect:
        for (int slash = 0; slash < 3; ++slash) {
            float grow = min(1.f, (progress - 0.3f - slash * 0.12f) / 0.35f);
            if (grow <= 0.f) continue;
            sf::Vector2f slashstart = target + sf::Vector2f(-45.f + slash * 45.f - 45.f, -85.f);
            sf::Vector2f slashend = slashstart + sf::Vector2f(90.f, 170.f) * grow;
            sf::Uint8 alpha = progress > 0.8f ? (sf::Uint8)(255.f * (1.f - (progress - 0.8f) / 0.2f)) : 255;
            drawline(canvas, slashstart, slashend, 12.f, sf::Color(130, 60, 255, alpha));
            drawline(canvas, slashstart, slashend, 4.f, sf::Color(255, 255, 255, alpha));
        }
        break;
    case SlashEffect: {
        sf::Vector2f direction = target - origin;
        float angle = atan2(direction.y, direction.x) * 180.f / pi;
        for (int blade = 0; blade < 3; ++blade) {
            sf::CircleShape crescent(46.f);
            crescent.setOrigin(46.f, 46.f); crescent.setScale(0.3f, 1.f); crescent.setRotation(angle);
            crescent.setPosition(blend(origin, target, max(0.f, progress - 0.08f * blade)));
            crescent.setFillColor(sf::Color(200, 235, 255, (sf::Uint8)(220 - 70 * blade)));
            canvas.draw(crescent);
        }
        break;
    }
    case QuakeEffect: {
        sf::Vector2f groundstart = origin + sf::Vector2f(0.f, 85.f), groundend = target + sf::Vector2f(0.f, 80.f);
        const int segments = 12;
        for (int segment = 0; segment < segments; ++segment) {
            float startfraction = (float)segment / segments, endfraction = (float)(segment + 1) / segments;
            if (startfraction > progress) break;
            sf::Vector2f segmentstart = blend(groundstart, groundend, startfraction) + sf::Vector2f(0.f, (noise(segment, 3) - 0.5f) * 26.f);
            sf::Vector2f segmentend = blend(groundstart, groundend, endfraction) + sf::Vector2f(0.f, (noise(segment + 1, 3) - 0.5f) * 26.f);
            drawline(canvas, segmentstart, segmentend, 9.f, sf::Color(60, 40, 20));
            drawline(canvas, segmentstart, segmentend, 4.f, sf::Color(150, 105, 55));
            drawdot(canvas, segmentstart + sf::Vector2f(0.f, -10.f - 20.f * (progress - startfraction)), 7.f + 6.f * noise(segment, 5), sf::Color(205, 175, 125, 150));
        }
        break;
    }
    case LeafEffect:
        for (int i = 0; i < 7; ++i) {
            float along = progress * 1.2f - i * 0.07f;
            if (along <= 0.f || along > 1.f) continue;
            sf::Vector2f position = blend(origin, target, along) + sf::Vector2f(0.f, sin(along * 9.f + i) * 26.f);
            sf::CircleShape leaf(15.f);
            leaf.setOrigin(15.f, 15.f); leaf.setScale(1.f, 0.38f);
            leaf.setRotation(along * 720.f + i * 50.f); leaf.setPosition(position);
            leaf.setFillColor(sf::Color((sf::Uint8)(70 + i * 12), (sf::Uint8)(190 + i * 6), 80));
            leaf.setOutlineColor(sf::Color(30, 100, 40)); leaf.setOutlineThickness(1.5f);
            canvas.draw(leaf);
        }
        break;
    case PulseEffect: {
        sf::Vector2f position = blend(origin, target, progress);
        float pulse = 1.f + 0.12f * sin(elapsed * 30.f);
        drawdot(canvas, position, 38.f * pulse, sf::Color(80, 30, 200, 120));
        drawdot(canvas, position, 26.f * pulse, sf::Color(150, 90, 255, 200));
        drawdot(canvas, position, 13.f * pulse, sf::Color(235, 220, 255, 255));
        if (progress > 0.8f) {
            float spread = (progress - 0.8f) / 0.2f;
            sf::CircleShape ring(30.f + 70.f * spread);
            ring.setOrigin(ring.getRadius(), ring.getRadius()); ring.setPosition(target);
            ring.setFillColor(sf::Color::Transparent);
            ring.setOutlineColor(sf::Color(170, 120, 255, (sf::Uint8)(220 * (1.f - spread)))); ring.setOutlineThickness(6.f);
            canvas.draw(ring);
        }
        break;
    }
    case StreakEffect: {
        for (int line = 0; line < 5; ++line) {
            float offset = (line - 2) * 15.f;
            sf::Vector2f head = blend(origin, target, progress) + sf::Vector2f(0.f, offset);
            sf::Vector2f tail = blend(origin, target, max(0.f, progress - 0.4f)) + sf::Vector2f(0.f, offset);
            drawline(canvas, tail, head, 4.f, sf::Color(255, 255, 255, (sf::Uint8)(230 * (1.f - 0.5f * progress))));
        }
        if (progress > 0.8f) for (int ray = 0; ray < 4; ++ray) {
            float angle = ray * pi / 4.f;
            drawline(canvas, target, target + sf::Vector2f(cos(angle), sin(angle)) * 45.f, 4.f, sf::Color(255, 255, 255, 230));
        }
        break;
    }
    case ThunderEffect: {
        if (progress < 0.3f) {
            drawdot(canvas, origin, 10.f + 10.f * noise((int)(elapsed * 40), 1), sf::Color(255, 235, 80, 230));
        } else if (fmod(progress * 14.f, 2.f) < 1.6f) {
            sf::Vector2f top = target + sf::Vector2f(0.f, -230.f);
            const int pieces = 9;
            sf::Vector2f previous = top;
            int seed = (int)(elapsed * 25.f);
            for (int piece = 1; piece <= pieces; ++piece) {
                sf::Vector2f point = blend(top, target, (float)piece / pieces);
                if (piece < pieces) point.x += (noise(piece, seed) - 0.5f) * 60.f;
                drawline(canvas, previous, point, 10.f, sf::Color(255, 225, 60, 200));
                drawline(canvas, previous, point, 3.f, sf::Color(255, 255, 255));
                previous = point;
            }
            drawdot(canvas, target, 55.f * (1.f - progress) + 15.f, sf::Color(255, 240, 120, 120));
        }
        break;
    }
    default: {
        sf::Vector2f position = blend(origin, target, progress);
        position.y -= sin(progress * pi) * 40.f;
        drawdot(canvas, position, 18.f, sf::Color(projectile.color.r, projectile.color.g, projectile.color.b, 230));
        drawdot(canvas, position, 8.f, sf::Color(255, 255, 255, 230));
        break;
    }
    }
}

// x, y = outer top-left; width and height are outer sizes, height INCLUDING the 4px dark rim at the bottom.
void BattleScene::drawhudbox(float x, float y, float width, float height, bool tailleft) {
    rect(x, y, width, height, shade(0x506860));
    rect(x + 1, y + 1, width - 2, height - 5, shade(0x203800));
    rect(x + 2, y + 2, width - 4, height - 7, shade(0xD8D0B0));
    rect(x + 3, y + 3, width - 6, height - 9, shade(0xF8F8D8));
    const float widths[4] = { 2.f, 4.f, 6.f, 8.f };
    for (int i = 0; i < 4; ++i) {
        float rowy = y + height - 4 + i;
        if (tailleft) rect(x - widths[i] + 1, rowy, widths[i], 1, shade(0x506860));
        else          rect(x + width - 1, rowy, widths[i], 1, shade(0x506860));
    }
}

void BattleScene::drawhealthbar(float x, float y, const ui::HealthBar& bar, StatusType condition) {
    if (condition == StatusType::None) {
        outlined("HP", x - 13.f, y + 7.f, smallsize, shade(0xF8B040), shade(0x506858));
    } else {
        rect(x - 14.f, y, 13.f, 7.f, condition == StatusType::Burn ? shade(0xF08030) : shade(0xF8D038));
        text(condition == StatusType::Burn ? "BRN" : "PAR", x - 13.f, y + 6.f, 7, shade(0xF8F8F8), shade(0x506858), false);
    }
    rect(x, y, 52.f, 7.f, shade(0x506858));
    rect(x + 1, y + 1, 50.f, 5.f, shade(0xF8F8F8));
    float fraction = bar.displayed();
    int filled = (int)ceil(fraction * 48.f - 0.001f);
    if (fraction > 0.f && filled < 1) filled = 1;
    sf::Color dark, light;
    if (fraction > 0.5f)      { dark = shade(0x58D080); light = shade(0x70F8A8); }
    else if (fraction > 0.2f) { dark = shade(0xC8A808); light = shade(0xF8E038); }
    else                      { dark = shade(0xA84048); light = shade(0xF85838); }
    if (filled > 0) {
        rect(x + 2, y + 2, (float)filled, 1, dark);
        rect(x + 2, y + 3, (float)filled, 2, light);
    }
}

void BattleScene::drawhud() {
    const Pokemon& playerone = battle.playerone();
    const Pokemon& playertwo = battle.playertwo();
    const sf::Color ink = shade(0x404040), inkshadow = shade(0xD8D0B0);

    drawhudbox(13.f, 16.f, 92.f, 29.f, false);
    text(uppercase(playertwo.name), 20.f, 28.f, hudsize, ink, inkshadow);
    string levellabel = "Lv" + to_string(playertwo.level);
    text(levellabel, 98.f - textwidth(levellabel, hudsize), 28.f, hudsize, ink, inkshadow);
    drawhealthbar(50.f, 31.f, healthbars[1], shownstatus[1]);

    drawhudbox(136.f, 74.f, 91.f, 37.f, true);
    text(uppercase(playerone.name), 143.f, 86.f, hudsize, ink, inkshadow);
    levellabel = "Lv" + to_string(playerone.level);
    text(levellabel, 222.f - textwidth(levellabel, hudsize), 86.f, hudsize, ink, inkshadow);
    drawhealthbar(173.f, 90.f, healthbars[0], shownstatus[0]);
    int currenthealth = (int)ceil(healthbars[0].displayed() * playerone.maxhealth - 0.001f);
    char buffer[24];
    snprintf(buffer, sizeof buffer, "%3d/%3d", currenthealth, playerone.maxhealth);
    text(buffer, 222.f - textwidth(buffer, hudsize), 103.f, hudsize, ink, inkshadow);
    outlined("EXP", 144.f, 111.f, 6, shade(0xF8E000), shade(0x506860));
    rect(157.f, 108.f, 69.f, 2.f, shade(0xC0B870));
}

void BattleScene::drawdialogframe() {
    const float width = 240.f;
    rect(0, 112, width, 48, shade(0x283030));
    rect(2, 114, width - 4, 44, shade(0xC8A848));
    rect(6, 117, width - 12, 38, shade(0xE0D8E0));
    rect(8, 119, width - 16, 34, shade(0x285068));
}

void BattleScene::drawmenuframe(float x, float width) {
    rect(x, 112, width, 48, shade(0x283030));
    rect(x + 2, 114, width - 4, 44, shade(0x706880));
    rect(x + 2, 114, width - 4, 1, shade(0x8888C8));
    rect(x + 2, 114, 1, 44, shade(0x8888C8));
    rect(x + width - 3, 114, 1, 44, shade(0x484868));
    rect(x + 2, 157, width - 4, 1, shade(0x484868));
    rect(x + 5, 117, width - 10, 38, shade(0xD8D0D8));
    rect(x + 6, 118, width - 12, 36, shade(0xF8F8F8));
}

void BattleScene::drawcursor(float x, float y) {
    static const int widths[8] = { 2, 3, 4, 5, 5, 4, 3, 2 };
    for (int i = 0; i < 8; ++i) rect(x, y + i, (float)widths[i], 1, shade(0x101010));
}

void BattleScene::drawbottom() {
    const sf::Color white = shade(0xF8F8F8), whiteshadow = shade(0x685870);
    const sf::Color ink = shade(0x484848), inkshadow = shade(0xD0D0C8);
    drawdialogframe();

    string lines[3];
    int linecount = 0;
    if (phase == Phase::Menu && mode == Mode::Command) {
        lines[0] = "What will";
        lines[1] = uppercase(battle.selectingside() == 2 ? battle.playertwo().name : battle.playerone().name) + " do?";
        linecount = 2;
    } else if (phase == Phase::Playing) {
        linecount = wrap(message, 222.f, bigsize, lines, 2);
    } else if (phase == Phase::GameOver) {
        linecount = wrap(message, 104.f, bigsize, lines, 2);
    }
    if (!(phase == Phase::Menu && mode == Mode::Moves))
        for (int i = 0; i < linecount; ++i) text(lines[i], 10.f, rowbaseline[i], bigsize, white, whiteshadow);

    if (phase == Phase::Playing) return;

    if (mode == Mode::Command) {
        drawmenuframe(120.f, 120.f);
        for (int i = 0; i < 4; ++i) {
            bool disabled = (i == 0 && !battle.awaitinginput());
            text(commandlabels[i], commandx[i], rowbaseline[i / 2], bigsize, disabled ? shade(0xA8A8A8) : ink, inkshadow);
        }
        drawcursor(commandx[commandchoice] - 7.f, rowbaseline[commandchoice / 2] - 9.f);
    } else {
        const bool firstside = battle.selectingside() != 2;
        const Pokemon& current = firstside ? battle.playerone() : battle.playertwo();
        drawmenuframe(0.f, 160.f);
        drawmenuframe(160.f, 80.f);
        unsigned movesize = bigsize;
        for (int i = 0; i < 4; ++i) {
            const Move* movedata = battle.movedatabase().get(current.moves[i]);
            if (movedata) while (movesize > 9 && textwidth(uppercase(movedata->name), movesize) > 66.f) --movesize;
        }
        for (int i = 0; i < 4; ++i) {
            const Move* movedata = battle.movedatabase().get(current.moves[i]);
            if (movedata) text(uppercase(movedata->name), movex[i % 2], rowbaseline[i / 2], movesize, ink, inkshadow);
        }
        drawcursor(movex[movechoice % 2] - 7.f, rowbaseline[movechoice / 2] - 9.f);
        const Move* selected = battle.movedatabase().get(current.moves[movechoice]);
        if (selected) {
            string typeline = string("TYPE/") + uppercase(nameoftype(selected->type));
            string statsline = "PWR" + to_string(selected->power) + " ACC" + to_string(selected->accuracy);
            unsigned infosize = bigsize;
            while (infosize > 9 && (textwidth(typeline, infosize) > 66.f || textwidth(statsline, infosize) > 66.f)) --infosize;
            text(typeline, 168.f, rowbaseline[0], infosize, ink, inkshadow);
            text(statsline, 168.f, rowbaseline[1], infosize, ink, inkshadow);
        }
    }
}

void BattleScene::render() {
    window.clear(sf::Color(0xF0, 0xF0, 0xF0));
    window.setView(window.getDefaultView());
    window.draw(backgroundsprite);

    sf::View view = window.getDefaultView();
    if (quake > 0.f) view.move(sin(elapsed * 90.f) * 9.f * quake, cos(elapsed * 70.f) * 6.f * quake);
    window.setView(view);
    drawpokemon(false);
    drawpokemon(true);
    drawprojectile();
    window.setView(window.getDefaultView());

    drawhud();
    drawbottom();
    window.display();
}

void BattleScene::run() {
    sf::Clock stopwatch;
    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) handleevent(event);
        float seconds = min(stopwatch.restart().asSeconds(), 0.05f);
        update(seconds);
        render();
    }
}
