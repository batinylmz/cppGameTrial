#include <SFML/Graphics.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <vector>
#include <cmath>
#include <optional>
#include <cstdlib>
#include <ctime>
#include <cstdint>

const float PI = 3.14159265f;
const float ZEMIN_Y = 500.f;

class OyunNesnesi {
public:
    virtual ~OyunNesnesi() {}
    virtual void guncelle() = 0;
    virtual void ciz(sf::RenderWindow& pencere) = 0;
    virtual sf::FloatRect getSinirlar() { return sf::FloatRect(); }
    virtual bool silinecekMi() { return false; }
};

// 1. PARTİKÜL SİSTEMİ
class Partikul : public OyunNesnesi {
private:
    sf::CircleShape sekil;
    float hizX, hizY;
    int omur, maxOmur;

public:
    Partikul(float x, float y, sf::Color renk, float sacilmaGucu = 10.f) {
        sekil.setPosition({x, y});
        sekil.setRadius(2.f + static_cast<float>(std::rand() % 4));
        sekil.setFillColor(renk);
        sekil.setOrigin({sekil.getRadius(), sekil.getRadius()});

        hizX = (std::rand() % static_cast<int>(sacilmaGucu * 2) - sacilmaGucu) / 2.0f;
        hizY = (std::rand() % static_cast<int>(sacilmaGucu * 2) - sacilmaGucu) / 2.0f;

        maxOmur = 15 + std::rand() % 15;
        omur = maxOmur;
    }

    void guncelle() override {
        sekil.move({hizX, hizY});
        hizY += 0.2f;
        omur--;

        sf::Color renk = sekil.getFillColor();
        renk.a = static_cast<std::uint8_t>((255.f * omur) / maxOmur);
        sekil.setFillColor(renk);
    }

    void ciz(sf::RenderWindow& pencere) override { pencere.draw(sekil); }
    bool silinecekMi() override { return omur <= 0; }
};

// 2. TANK MERMİSİ
class Mermi : public OyunNesnesi {
private:
    sf::CircleShape sekil;
    float hizX, hizY;
    const float YERCEKIMI = 0.3f;

public:
    Mermi(float x, float y, float aciDerece) {
        sekil.setRadius(6.f); sekil.setFillColor(sf::Color::Black);
        sekil.setOrigin({6.f, 6.f}); sekil.setPosition({x, y});

        float aciRadyan = aciDerece * PI / 180.f;
        hizX = 18.f * std::cos(aciRadyan);
        hizY = 18.f * std::sin(aciRadyan);
    }

    void guncelle() override {
        hizY += YERCEKIMI;
        sekil.move({hizX, hizY});
    }

    void ciz(sf::RenderWindow& pencere) override { pencere.draw(sekil); }
    sf::FloatRect getSinirlar() override { return sekil.getGlobalBounds(); }
    bool silinecekMi() override { return sekil.getPosition().y > 600.f; }
};

// 3. ADAMIN ROKETİ (SFML 3.0 sf::Angle Güncellemesi Yapıldı)
class Roket : public OyunNesnesi {
private:
    sf::RectangleShape govde, uc;
    sf::CircleShape ates;
    float hizX, hizY;
    const float YERCEKIMI = 0.15f;

public:
    Roket(float x, float y, float aciDerece) {
        govde.setSize({26.f, 10.f}); govde.setFillColor(sf::Color(80, 80, 80));
        govde.setOrigin({13.f, 5.f}); govde.setPosition({x, y});

        uc.setSize({10.f, 8.f}); uc.setFillColor(sf::Color::Red);
        uc.setOrigin({5.f, 4.f});

        ates.setRadius(5.f); ates.setFillColor(sf::Color::Yellow);
        ates.setOrigin({5.f, 5.f});

        float rad = aciDerece * PI / 180.f;
        float firlatmaHizi = 18.f;
        hizX = -firlatmaHizi * std::cos(rad);
        hizY = -firlatmaHizi * std::sin(rad);
    }

    void guncelle() override {
        hizY += YERCEKIMI;
        govde.move({hizX, hizY});

        float ucusAcisi = std::atan2(hizY, -hizX) * 180.f / PI;
        // DÜZELTME: Doğrudan float vermek yerine sf::degrees kullanıldı
        govde.setRotation(sf::degrees(ucusAcisi));

        if (std::rand() % 2 == 0) ates.setFillColor(sf::Color::Red);
        else ates.setFillColor(sf::Color::Yellow);
    }

    void ciz(sf::RenderWindow& pencere) override {
        // DÜZELTME: getRotation artık sf::Angle döndürüyor, asRadians() ile sayısal değere çevrildi
        float rad = govde.getRotation().asRadians();

        uc.setPosition({govde.getPosition().x - 13.f * std::cos(rad), govde.getPosition().y - 13.f * std::sin(rad)});
        uc.setRotation(govde.getRotation());

        ates.setPosition({govde.getPosition().x + 13.f * std::cos(rad), govde.getPosition().y + 13.f * std::sin(rad)});

        pencere.draw(ates); pencere.draw(govde); pencere.draw(uc);
    }
    sf::FloatRect getSinirlar() override { return govde.getGlobalBounds(); }
    bool silinecekMi() override { return govde.getPosition().x < -50.f || govde.getPosition().y > 600.f; }
};

// 4. TANK SINIFI
class Tank : public OyunNesnesi {
private:
    sf::RectangleShape govde, kule, namlu, hitbox;
    sf::CircleShape teker1, teker2, teker3;
    float namluAcisi = 0.f;
    int can = 3;
    int hasarSuresi = 0;

public:
    Tank(float x, float tabanY) {
        float tekerY = tabanY - 15.f;
        teker1.setRadius(15.f); teker1.setFillColor(sf::Color(30, 30, 30)); teker1.setPosition({x + 5.f, tekerY});
        teker2.setRadius(15.f); teker2.setFillColor(sf::Color(30, 30, 30)); teker2.setPosition({x + 45.f, tekerY});
        teker3.setRadius(15.f); teker3.setFillColor(sf::Color(30, 30, 30)); teker3.setPosition({x + 85.f, tekerY});

        govde.setSize({120.f, 30.f}); govde.setFillColor(sf::Color(34, 139, 34)); govde.setPosition({x, tabanY - 40.f});
        kule.setSize({60.f, 25.f}); kule.setFillColor(sf::Color(0, 100, 0)); kule.setPosition({x + 30.f, tabanY - 65.f});
        namlu.setSize({50.f, 8.f}); namlu.setFillColor(sf::Color::Black);
        namlu.setOrigin({0.f, 4.f}); namlu.setPosition({x + 90.f, tabanY - 52.f});

        hitbox.setSize({120.f, 65.f});
        hitbox.setOrigin({0.f, 0.f});
        hitbox.setPosition({x, tabanY - 65.f});
    }

    void guncelle() override {
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D)) hareketEttir(4.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A)) hareketEttir(-4.f);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W)) {
            namluAcisi -= 1.5f; if (namluAcisi < -60.f) namluAcisi = -60.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S)) {
            namluAcisi += 1.5f; if (namluAcisi > 0.f) namluAcisi = 0.f;
        }
        namlu.setRotation(sf::degrees(namluAcisi));

        if (hasarSuresi > 0) {
            hasarSuresi--;
            govde.setFillColor(sf::Color::White); kule.setFillColor(sf::Color::White);
        } else {
            govde.setFillColor(sf::Color(34, 139, 34)); kule.setFillColor(sf::Color(0, 100, 0));
        }
    }

    void hareketEttir(float hizX) {
        govde.move({hizX, 0.f}); kule.move({hizX, 0.f}); namlu.move({hizX, 0.f});
        teker1.move({hizX, 0.f}); teker2.move({hizX, 0.f}); teker3.move({hizX, 0.f});
        hitbox.move({hizX, 0.f});
    }

    void ciz(sf::RenderWindow& pencere) override {
        pencere.draw(govde); pencere.draw(kule); pencere.draw(namlu);
        pencere.draw(teker1); pencere.draw(teker2); pencere.draw(teker3);
    }

    sf::FloatRect getSinirlar() override { return hitbox.getGlobalBounds(); }
    float getNamluAcisi() { return namluAcisi; }
    sf::Vector2f getNamluUcuKonumu() {
        float aciRadyan = namluAcisi * PI / 180.f;
        return {namlu.getPosition().x + (namlu.getSize().x * std::cos(aciRadyan)),
                namlu.getPosition().y + (namlu.getSize().x * std::sin(aciRadyan))};
    }

    void hasarAl() { if (can > 0) { can--; hasarSuresi = 15; } }
    int getCan() { return can; }
};

// 5. HEDEF ADAM SINIFI (Silah Açısı O ve L Tuşları İle Ayarlandı)
class HedefAdam : public OyunNesnesi {
private:
    sf::CircleShape kafa;
    sf::RectangleShape govde, onKol, arkaKol, solBacak, sagBacak, hitbox, bazuka;
    int can = 3;
    int yanmaSuresi = 0;
    float hizY = 0.f;
    bool yerdeMi = true;
    float silahAcisi = 0.f;

public:
    HedefAdam(float x, float tabanY) {
        solBacak.setSize({6.f, 30.f}); solBacak.setOrigin({3.f, 0.f}); solBacak.setPosition({x - 5.f, tabanY - 30.f});
        sagBacak.setSize({6.f, 30.f}); sagBacak.setOrigin({3.f, 0.f}); sagBacak.setPosition({x + 5.f, tabanY - 30.f});
        govde.setSize({16.f, 35.f}); govde.setOrigin({8.f, 0.f}); govde.setPosition({x, tabanY - 65.f});
        kafa.setRadius(12.f); kafa.setOrigin({12.f, 12.f}); kafa.setPosition({x, tabanY - 77.f});

        bazuka.setSize({55.f, 14.f});
        bazuka.setFillColor(sf::Color(47, 79, 79));
        bazuka.setOutlineThickness(1.f);
        bazuka.setOutlineColor(sf::Color::Black);
        bazuka.setOrigin({27.5f, 7.f});
        bazuka.setPosition({x - 15.f, tabanY - 53.f});

        onKol.setSize({25.f, 6.f}); onKol.setOrigin({25.f, 3.f});
        onKol.setPosition({x - 5.f, tabanY - 53.f});
        arkaKol.setSize({15.f, 6.f}); arkaKol.setOrigin({0.f, 3.f});
        arkaKol.setPosition({x, tabanY - 53.f});

        renkAyarlaNormale();
        hitbox.setSize({55.f, 90.f}); hitbox.setOrigin({27.5f, 90.f}); hitbox.setPosition({x, tabanY});
    }

    void renkAyarlaNormale() {
        kafa.setFillColor(sf::Color(255, 224, 189)); govde.setFillColor(sf::Color(65, 105, 225));
        onKol.setFillColor(sf::Color(65, 105, 225)); arkaKol.setFillColor(sf::Color(65, 105, 225));
        solBacak.setFillColor(sf::Color(139, 69, 19)); sagBacak.setFillColor(sf::Color(139, 69, 19));
    }

    void renkAyarlaYanan() {
        sf::Color ates(255, 69, 0);
        kafa.setFillColor(ates); govde.setFillColor(ates);
        onKol.setFillColor(ates); arkaKol.setFillColor(ates);
        solBacak.setFillColor(ates); sagBacak.setFillColor(ates);
    }

    void guncelle() override {
        float hizX = 0.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) hizX = 4.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) hizX = -4.f;
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up) && yerdeMi) { hizY = -12.f; yerdeMi = false; }

        // DÜZELTME: Klavye sorunu yaşamamak için O ve L kullanıldı
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::O)) {
            silahAcisi += 1.5f; if (silahAcisi > 60.f) silahAcisi = 60.f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L)) {
            silahAcisi -= 1.5f; if (silahAcisi < -15.f) silahAcisi = -15.f;
        }

        // DÜZELTME: sf::degrees kullanıldı
        bazuka.setRotation(sf::degrees(silahAcisi));

        hizY += 0.5f;
        hareketEttir(hizX, hizY);

        if (hitbox.getPosition().y > ZEMIN_Y) {
            hareketEttir(0.f, ZEMIN_Y - hitbox.getPosition().y);
            hizY = 0.f; yerdeMi = true;
        }

        if (yanmaSuresi > 0) { yanmaSuresi--; renkAyarlaYanan(); }
        else { renkAyarlaNormale(); }
    }

    void hareketEttir(float dx, float dy) {
        kafa.move({dx, dy}); govde.move({dx, dy}); bazuka.move({dx, dy});
        onKol.move({dx, dy}); arkaKol.move({dx, dy});
        solBacak.move({dx, dy}); sagBacak.move({dx, dy});
        hitbox.move({dx, dy});
    }

    void yereBas() { hizY = 0.f; yerdeMi = true; }

    void ciz(sf::RenderWindow& pencere) override {
        pencere.draw(arkaKol); pencere.draw(govde); pencere.draw(kafa);
        pencere.draw(solBacak); pencere.draw(sagBacak);
        pencere.draw(bazuka); pencere.draw(onKol);
    }

    sf::FloatRect getSinirlar() override { return hitbox.getGlobalBounds(); }

    sf::Vector2f getSilahUcu() {
        float rad = silahAcisi * PI / 180.f;
        return {bazuka.getPosition().x - 27.5f * std::cos(rad),
                bazuka.getPosition().y - 27.5f * std::sin(rad)};
    }

    float getSilahAcisi() { return silahAcisi; }
    void hasarAl() { if (can > 0) { can--; yanmaSuresi = 20; } }
    int getCan() { return can; }
};

int main() {
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    sf::RenderWindow window(sf::VideoMode({800, 600}), "Muhtesem Tank vs Adam Savasi");
    window.setFramerateLimit(60);

    sf::RectangleShape cimenZemin({800.f, 100.f});
    cimenZemin.setFillColor(sf::Color(85, 170, 85));
    cimenZemin.setPosition({0.f, ZEMIN_Y});

    sf::Font font;
    bool fontYuklendi = font.openFromFile("/System/Library/Fonts/Supplemental/Arial.ttf");
    std::optional<sf::Text> gameOverYazisi;

    sf::RectangleShape karartma({800.f, 600.f});
    karartma.setFillColor(sf::Color(0, 0, 0, 180));

    std::vector<OyunNesnesi*> nesneler;
    Tank* oyuncu1 = new Tank(50.f, ZEMIN_Y);
    HedefAdam* oyuncu2 = new HedefAdam(650.f, ZEMIN_Y);
    nesneler.push_back(oyuncu1);
    nesneler.push_back(oyuncu2);

    bool oyunBitti = false;
    int ekranTitremeSuresi = 0;
    sf::View varsayilanGorus = window.getDefaultView();

    while (window.isOpen()) {
        std::vector<OyunNesnesi*> guncelEklenecekler;

        while (const std::optional<sf::Event> event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) window.close();

            if (!oyunBitti) {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::Space) {
                        sf::Vector2f ucu = oyuncu1->getNamluUcuKonumu();
                        guncelEklenecekler.push_back(new Mermi(ucu.x, ucu.y, oyuncu1->getNamluAcisi()));
                        for(int i=0; i<10; i++) guncelEklenecekler.push_back(new Partikul(ucu.x, ucu.y, sf::Color(255, 165, 0), 5.f));
                    }
                    if (key->code == sf::Keyboard::Key::Enter) {
                        sf::Vector2f ucu = oyuncu2->getSilahUcu();
                        guncelEklenecekler.push_back(new Roket(ucu.x, ucu.y, oyuncu2->getSilahAcisi()));
                        for(int i=0; i<10; i++) guncelEklenecekler.push_back(new Partikul(ucu.x, ucu.y, sf::Color::Yellow, 8.f));
                    }
                }
            } else {
                if (const auto* key = event->getIf<sf::Event::KeyPressed>()) {
                    if (key->code == sf::Keyboard::Key::Enter || key->code == sf::Keyboard::Key::Space) {
                        for (OyunNesnesi* nesne : nesneler) delete nesne;
                        nesneler.clear();
                        oyuncu1 = new Tank(50.f, ZEMIN_Y);
                        oyuncu2 = new HedefAdam(650.f, ZEMIN_Y);
                        nesneler.push_back(oyuncu1); nesneler.push_back(oyuncu2);
                        oyunBitti = false;
                        ekranTitremeSuresi = 0;
                        window.setView(varsayilanGorus);
                    }
                }
            }
        }

        if (ekranTitremeSuresi > 0) {
            float dx = (std::rand() % 10 - 5) * (ekranTitremeSuresi / 20.f);
            float dy = (std::rand() % 10 - 5) * (ekranTitremeSuresi / 20.f);
            sf::View sarsilmisGorus = varsayilanGorus;
            sarsilmisGorus.move({dx, dy});
            window.setView(sarsilmisGorus);
            ekranTitremeSuresi--;
        } else {
            window.setView(varsayilanGorus);
        }

        window.clear(sf::Color(135, 206, 235));
        window.draw(cimenZemin);

        if (!oyunBitti) {
            for (auto it = nesneler.begin(); it != nesneler.end(); ) {
                OyunNesnesi* nesne = *it;
                nesne->guncelle();

                if (nesne->silinecekMi()) {
                    delete nesne;
                    it = nesneler.erase(it);
                    continue;
                }

                Mermi* mermi = dynamic_cast<Mermi*>(nesne);
                if (mermi && mermi->getSinirlar().findIntersection(oyuncu2->getSinirlar())) {
                    oyuncu2->hasarAl();
                    ekranTitremeSuresi = 15;
                    for(int i=0; i<20; i++) guncelEklenecekler.push_back(new Partikul(mermi->getSinirlar().position.x, mermi->getSinirlar().position.y, sf::Color::Red, 12.f));
                    delete mermi;
                    it = nesneler.erase(it);
                    continue;
                }

                Roket* roket = dynamic_cast<Roket*>(nesne);
                if (roket && roket->getSinirlar().findIntersection(oyuncu1->getSinirlar())) {
                    oyuncu1->hasarAl();
                    ekranTitremeSuresi = 30;
                    for(int i=0; i<40; i++) guncelEklenecekler.push_back(new Partikul(roket->getSinirlar().position.x, roket->getSinirlar().position.y, sf::Color(255, 100, 0), 20.f));
                    delete roket;
                    it = nesneler.erase(it);
                    continue;
                }

                ++it;
            }

            for (OyunNesnesi* yeni : guncelEklenecekler) {
                nesneler.push_back(yeni);
            }

            std::optional<sf::FloatRect> kesisim = oyuncu1->getSinirlar().findIntersection(oyuncu2->getSinirlar());
            if (kesisim) {
                if (kesisim->size.x < kesisim->size.y) {
                    if (oyuncu2->getSinirlar().position.x < oyuncu1->getSinirlar().position.x) oyuncu2->hareketEttir(-kesisim->size.x, 0.f);
                    else oyuncu2->hareketEttir(kesisim->size.x, 0.f);
                } else {
                    if (oyuncu2->getSinirlar().position.y < oyuncu1->getSinirlar().position.y) {
                        oyuncu2->hareketEttir(0.f, -kesisim->size.y); oyuncu2->yereBas();
                    }
                }
            }

            if (oyuncu1->getCan() <= 0 || oyuncu2->getCan() <= 0) {
                oyunBitti = true;
                if (fontYuklendi) {
                    std::string kazananMesaj = (oyuncu1->getCan() > 0) ? "KAZANAN: TANK" : "KAZANAN: ADAM";
                    gameOverYazisi.emplace(font, "OYUN BITTI!\n" + kazananMesaj + "\n\nTekrar Oynamak Icin\nENTER Tusuna Bas", 40);
                    gameOverYazisi->setFillColor(sf::Color::White);
                    gameOverYazisi->setStyle(sf::Text::Bold);
                    sf::FloatRect yaziSinir = gameOverYazisi->getLocalBounds();
                    gameOverYazisi->setOrigin({yaziSinir.position.x + yaziSinir.size.x / 2.0f, yaziSinir.position.y + yaziSinir.size.y / 2.0f});
                    gameOverYazisi->setPosition({400.f, 250.f});
                }
            }
        }

        for (OyunNesnesi* nesne : nesneler) nesne->ciz(window);

        for (int i = 0; i < oyuncu1->getCan(); i++) {
            sf::RectangleShape tankCan({20.f, 20.f});
            tankCan.setFillColor(sf::Color::Green);
            tankCan.setOutlineThickness(2.f); tankCan.setOutlineColor(sf::Color::White);
            tankCan.setPosition({20.f + (i * 30.f), 20.f});
            window.draw(tankCan);
        }

        for (int i = 0; i < oyuncu2->getCan(); i++) {
            sf::CircleShape adamCan(10.f);
            adamCan.setFillColor(sf::Color::Red);
            adamCan.setOutlineThickness(2.f); adamCan.setOutlineColor(sf::Color::White);
            adamCan.setPosition({750.f - (i * 30.f), 20.f});
            window.draw(adamCan);
        }

        if (oyunBitti) {
            window.draw(karartma);
            if (gameOverYazisi) window.draw(*gameOverYazisi);
        }

        window.display();
    }

    for (OyunNesnesi* nesne : nesneler) delete nesne;
    nesneler.clear();

    return 0;
}