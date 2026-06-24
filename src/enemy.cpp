#include "enemy.hpp"
#include <cmath>
#include <algorithm>
//Enemy
Enemy::Enemy(sf::RenderWindow& window, Type type, float spawnX, float groundY,
             float targetHitTime, float holdDuration)
: m_window(window)
, m_type(type)
, m_groundY(groundY)
, m_targetHitTime(targetHitTime)
, m_holdDuration(holdDuration)
{
    setupShape();
    float y = groundY - m_size;
    if (m_type == Type::A) {
        m_circle.setPosition(spawnX, y);
    } else {
        m_box.setPosition(spawnX, y - (m_size * 2.5f + 20.f));
    }
    updateTail();
}

void Enemy::setupShape()
{
    if (m_type == Type::A) {
        m_circle.setRadius(m_size / 2.f);
        m_circle.setOrigin(m_size / 2.f, m_size / 2.f);
        m_circle.setFillColor(sf::Color(0, 220, 255, 220));
        m_circle.setOutlineColor(sf::Color(140, 255, 255, 255));
        m_circle.setOutlineThickness(4.f);
        m_circle.setPointCount(40);
    } else {
        m_box.setSize({m_size, m_size});
        m_box.setOrigin(m_size / 2.f, m_size / 2.f);
        m_box.setFillColor(sf::Color(255, 0, 200, 220));
        m_box.setOutlineColor(sf::Color(255, 140, 255, 255));
        m_box.setOutlineThickness(4.f);
    }

    // Ekor hold note: garis tipis dari posisi shape ke arah kanan (spawn),
    // panjang awalnya 0 — baru memanjang ketika holdDuration > 0.
    m_tail.setSize({0.f, m_size * 0.35f});
    m_tail.setOrigin(0.f, m_tail.getSize().y / 2.f);
    m_tail.setFillColor(m_tailBaseColor);
}
void Enemy::setColor(sf::Color color)
{
    sf::Color outline(
        std::min(255, color.r + 80),
        std::min(255, color.g + 80),
        std::min(255, color.b + 80),
        255
    );
    if (m_type == Type::A) {
        m_circle.setFillColor(sf::Color(color.r, color.g, color.b, 220));
        m_circle.setOutlineColor(outline);
    } else {
        m_box.setFillColor(sf::Color(color.r, color.g, color.b, 220));
        m_box.setOutlineColor(outline);
    }

    // Ekor mengikuti warna utama tapi lebih redup
    m_tailBaseColor = sf::Color(color.r, color.g, color.b, 110);
    m_tail.setFillColor(m_tailBaseColor);
}

sf::FloatRect Enemy::getBounds() const
{
    if (m_type == Type::A) return m_circle.getGlobalBounds();
    return m_box.getGlobalBounds();
}

void Enemy::update(float dt)
{
    if (m_dead) return;

    if (m_type == Type::A) m_circle.move(-m_speed * dt, 0.f);
    else                    m_box.move(-m_speed * dt, 0.f);

    updateGlow(dt);
    updateTail();

    float rightEdge = getBounds().left + getBounds().width;
    if (rightEdge < 0.f) {
        m_passed = true;
    }
}

void Enemy::updateGlow(float dt)
{
    m_pulseTimer += dt;
    float pulse = (std::sin(m_pulseTimer * 8.f) + 1.f) * 0.5f;

    float thickness = 3.f + pulse * 3.f;
    sf::Uint8 glowAlpha = sf::Uint8(180 + pulse * 75.f);

    if (m_type == Type::A) {
        m_circle.setOutlineThickness(thickness);
        sf::Color oc = m_circle.getOutlineColor();
        oc.a = glowAlpha;
        m_circle.setOutlineColor(oc);
    } else {
        m_box.setOutlineThickness(thickness);
        sf::Color oc = m_box.getOutlineColor();
        oc.a = glowAlpha;
        m_box.setOutlineColor(oc);
    }
}

void Enemy::updateTail()
{
    if (m_holdDuration <= 0.f) return;   // tap note biasa, tidak ada ekor

    // Panjang ekor merepresentasikan SISA waktu hold yang masih harus ditahan,
    // dikonversi ke jarak pixel berdasarkan kecepatan musuh. Semakin lama
    // holdDuration, semakin panjang ekornya (mengarah ke kanan / arah spawn).
    float remainingHold = m_holdDuration;
    if (m_holdState == HoldState::Holding) {
        remainingHold = std::max(0.f, m_holdDuration - m_holdElapsed);
    } else if (m_holdState == HoldState::HoldSuccess || m_holdState == HoldState::HoldFailed) {
        remainingHold = 0.f;   // hold sudah berakhir, ekor menghilang
    }

    float tailLength = remainingHold * m_speed;

    sf::Vector2f center = (m_type == Type::A) ? m_circle.getPosition() : m_box.getPosition();
    m_tail.setSize({tailLength, m_tail.getSize().y});
    m_tail.setPosition(center);   // origin sudah di (0, height/2), jadi otomatis ke kanan

    // Ekor sedikit berkedip kalau sedang ditahan, supaya jelas "aktif"
    sf::Color c = m_tailBaseColor;
    if (m_holdState == HoldState::Holding) {
        float flicker = (std::sin(m_pulseTimer * 10.f) + 1.f) * 0.5f;
        c.a = sf::Uint8(140 + flicker * 100.f);
    }
    m_tail.setFillColor(c);
}
void Enemy::kill()
{
    m_dead = true;
}
void Enemy::draw()
{
    if (m_dead) return;
    sf::RenderStates rs;
    rs.blendMode = sf::BlendAlpha;

    // Ekor digambar DI BELAKANG shape utama (supaya shape tetap terlihat solid)
    if (m_holdDuration > 0.f && m_holdState != HoldState::HoldSuccess
        && m_holdState != HoldState::HoldFailed) {
        m_window.draw(m_tail, rs);
    }

    if (m_type == Type::A) m_window.draw(m_circle, rs);
    else                    m_window.draw(m_box, rs);
}
void Enemy::startHold(float stageTimerNow)
{
    if (m_holdDuration <= 0.f) return;   // bukan hold note, no-op
    if (m_holdState != HoldState::Idle) return;   // sudah pernah mulai, jangan restart

    m_holdState     = HoldState::Holding;
    m_holdElapsed   = 0.f;
    m_holdStartTime = stageTimerNow;
}

bool Enemy::updateHold(float dt, float stageTimerNow)
{
    if (m_holdState != HoldState::Holding) return false;

    m_holdElapsed += dt;
    updateTail();

    if (m_holdElapsed >= m_holdDuration) {
        m_holdState = HoldState::HoldSuccess;
        return true;   // hold selesai penuh, sambil masih ditahan
    }
    return false;
}

float Enemy::releaseHold()
{
    if (m_holdState != HoldState::Holding) return 0.f;

    float ratio = (m_holdDuration > 0.f)
        ? std::clamp(m_holdElapsed / m_holdDuration, 0.f, 1.f)
        : 1.f;

    m_holdState = HoldState::HoldFailed;
    return ratio;
}
namespace Beatmaps {

// EASY: 1 note tiap 4 beat (~1.82s), A/B bergantian — pemanasan, jarang.
// Setiap note ke-6 diganti jadi hold note 1 beat (~0.4545s) supaya pemain
// baru mulai kenal mekanik hold tanpa terlalu menumpuk.
std::vector<BeatNote> getEasy()
{
    return {
        { 3.636f, Enemy::Type::A },
        { 5.455f, Enemy::Type::B },
        { 7.273f, Enemy::Type::A },
        { 9.091f, Enemy::Type::B },
        { 10.909f, Enemy::Type::A },
        { 12.727f, Enemy::Type::B, 0.4545f },   // hold 1 beat
        { 14.545f, Enemy::Type::A },
        { 16.364f, Enemy::Type::B },
        { 18.182f, Enemy::Type::A },
        { 20.000f, Enemy::Type::B },
        { 21.818f, Enemy::Type::A },
        { 23.636f, Enemy::Type::B },
        { 25.455f, Enemy::Type::A, 0.4545f },   // hold 1 beat
        { 27.273f, Enemy::Type::B },
        { 29.091f, Enemy::Type::A },
        { 30.909f, Enemy::Type::B },
        { 32.727f, Enemy::Type::A },
        { 34.545f, Enemy::Type::B },
        { 36.364f, Enemy::Type::A },
        { 38.182f, Enemy::Type::B, 0.909f },    // hold 2 beat
        { 40.000f, Enemy::Type::A },
        { 41.818f, Enemy::Type::B },
        { 43.636f, Enemy::Type::A },
        { 45.455f, Enemy::Type::B },
        { 47.273f, Enemy::Type::A },
        { 49.091f, Enemy::Type::B },
        { 50.909f, Enemy::Type::A, 0.4545f },
        { 52.727f, Enemy::Type::B },
        { 54.545f, Enemy::Type::A },
        { 56.364f, Enemy::Type::B },
        { 58.182f, Enemy::Type::A },
        { 60.000f, Enemy::Type::B },
        { 61.818f, Enemy::Type::A },
        { 63.636f, Enemy::Type::B, 0.909f },
        { 65.455f, Enemy::Type::A },
        { 67.273f, Enemy::Type::B },
        { 69.091f, Enemy::Type::A },
        { 70.909f, Enemy::Type::B },
        { 72.727f, Enemy::Type::A },
        { 74.545f, Enemy::Type::B },
        { 76.364f, Enemy::Type::A, 0.4545f },
        { 78.182f, Enemy::Type::B },
        { 80.000f, Enemy::Type::A },
        { 81.818f, Enemy::Type::B },
        { 83.636f, Enemy::Type::A },
        { 85.455f, Enemy::Type::B },
        { 87.273f, Enemy::Type::A },
        { 89.091f, Enemy::Type::B, 0.909f },
        { 90.909f, Enemy::Type::A },
        { 92.727f, Enemy::Type::B },
        { 94.545f, Enemy::Type::A },
        { 96.364f, Enemy::Type::B },
        { 98.182f, Enemy::Type::A },
        { 100.000f, Enemy::Type::B },
        { 101.818f, Enemy::Type::A, 0.4545f },
        { 103.636f, Enemy::Type::B },
        { 105.455f, Enemy::Type::A },
        { 107.273f, Enemy::Type::B },
        { 109.091f, Enemy::Type::A },
        { 110.909f, Enemy::Type::B },
        { 112.727f, Enemy::Type::A },
        { 114.545f, Enemy::Type::B, 0.909f },
        { 116.364f, Enemy::Type::A },
        { 118.182f, Enemy::Type::B },
        { 120.000f, Enemy::Type::A },
        { 121.818f, Enemy::Type::B },
        { 123.636f, Enemy::Type::A },
        { 125.455f, Enemy::Type::B },
        { 127.273f, Enemy::Type::A, 0.4545f },
        { 129.091f, Enemy::Type::B },
        { 130.909f, Enemy::Type::A },
        { 132.727f, Enemy::Type::B },
        { 134.545f, Enemy::Type::A },
        { 136.364f, Enemy::Type::B },
        { 138.182f, Enemy::Type::A },
        { 140.000f, Enemy::Type::B, 0.909f },
        { 141.818f, Enemy::Type::A },
        { 143.636f, Enemy::Type::B },
        { 145.455f, Enemy::Type::A },
        { 147.273f, Enemy::Type::B },
        { 149.091f, Enemy::Type::A },
        { 150.909f, Enemy::Type::B },
        { 152.727f, Enemy::Type::A, 0.4545f },
        { 154.545f, Enemy::Type::B },
        { 156.364f, Enemy::Type::A },
        { 158.182f, Enemy::Type::B },
        { 160.000f, Enemy::Type::A },
        { 161.818f, Enemy::Type::B },
        { 163.636f, Enemy::Type::A },
        { 165.455f, Enemy::Type::B, 0.909f },
        { 167.273f, Enemy::Type::A },
        { 169.091f, Enemy::Type::B },
    };
}

// MEDIUM: 1 note tiap 2 beat (~0.91s), sesekali ada pasangan rapat (1 beat).
// Hold note muncul lebih sering (~setiap 8-10 note), durasi 1-2 beat,
// dan ditaruh di posisi yang TIDAK bertabrakan dengan pasangan rapat.
std::vector<BeatNote> getMedium()
{
    return {
        { 1.818f, Enemy::Type::A },
        { 2.727f, Enemy::Type::B },
        { 3.636f, Enemy::Type::A },
        { 4.545f, Enemy::Type::B },
        { 5.000f, Enemy::Type::A },
        { 5.909f, Enemy::Type::A, 0.909f },     // hold 2 beat
        { 7.727f, Enemy::Type::B },
        { 8.636f, Enemy::Type::A },
        { 9.545f, Enemy::Type::B },
        { 10.455f, Enemy::Type::A },
        { 11.364f, Enemy::Type::B },
        { 12.273f, Enemy::Type::A },
        { 13.182f, Enemy::Type::B },
        { 14.091f, Enemy::Type::A },
        { 15.000f, Enemy::Type::B, 0.4545f },   // hold 1 beat
        { 15.909f, Enemy::Type::A },
        { 16.818f, Enemy::Type::B },
        { 17.273f, Enemy::Type::A },
        { 18.182f, Enemy::Type::A },
        { 19.091f, Enemy::Type::B },
        { 20.000f, Enemy::Type::A },
        { 20.909f, Enemy::Type::B },
        { 21.364f, Enemy::Type::A },
        { 22.273f, Enemy::Type::A },
        { 23.182f, Enemy::Type::B },
        { 24.091f, Enemy::Type::A, 0.909f },
        { 25.909f, Enemy::Type::B },
        { 26.364f, Enemy::Type::A },
        { 27.273f, Enemy::Type::B },
        { 28.182f, Enemy::Type::A },
        { 29.091f, Enemy::Type::B },
        { 29.545f, Enemy::Type::A },
        { 30.455f, Enemy::Type::A },
        { 31.364f, Enemy::Type::B },
        { 32.273f, Enemy::Type::A },
        { 33.182f, Enemy::Type::B, 0.4545f },
        { 34.091f, Enemy::Type::A },
        { 34.545f, Enemy::Type::A },
        { 35.455f, Enemy::Type::B },
        { 36.364f, Enemy::Type::A },
        { 37.273f, Enemy::Type::B },
        { 37.727f, Enemy::Type::A },
        { 38.636f, Enemy::Type::A },
        { 39.545f, Enemy::Type::B },
        { 40.455f, Enemy::Type::A },
        { 41.364f, Enemy::Type::B },
        { 41.818f, Enemy::Type::A },
        { 42.727f, Enemy::Type::A, 0.909f },
        { 44.545f, Enemy::Type::B },
        { 45.455f, Enemy::Type::B },
        { 45.909f, Enemy::Type::A },
        { 46.818f, Enemy::Type::A },
        { 47.727f, Enemy::Type::B },
        { 48.636f, Enemy::Type::A },
        { 49.545f, Enemy::Type::B },
        { 50.000f, Enemy::Type::A },
        { 50.909f, Enemy::Type::A },
        { 51.818f, Enemy::Type::B },
        { 52.727f, Enemy::Type::A },
        { 53.636f, Enemy::Type::B, 0.4545f },
        { 54.545f, Enemy::Type::A },
        { 55.000f, Enemy::Type::A },
        { 55.909f, Enemy::Type::B },
        { 56.818f, Enemy::Type::A },
        { 57.727f, Enemy::Type::B },
        { 58.182f, Enemy::Type::A },
        { 59.091f, Enemy::Type::A },
        { 60.000f, Enemy::Type::B },
        { 60.909f, Enemy::Type::A },
        { 61.818f, Enemy::Type::B },
        { 62.273f, Enemy::Type::A },
        { 63.182f, Enemy::Type::A, 0.909f },
        { 65.000f, Enemy::Type::B },
        { 65.909f, Enemy::Type::B },
        { 66.364f, Enemy::Type::A },
        { 67.273f, Enemy::Type::A },
        { 68.182f, Enemy::Type::B },
        { 69.091f, Enemy::Type::A },
        { 70.000f, Enemy::Type::B },
        { 70.455f, Enemy::Type::A },
        { 71.364f, Enemy::Type::A },
        { 72.273f, Enemy::Type::B },
        { 73.182f, Enemy::Type::A },
        { 74.091f, Enemy::Type::B, 0.4545f },
        { 75.000f, Enemy::Type::A },
        { 75.455f, Enemy::Type::A },
        { 76.364f, Enemy::Type::B },
        { 77.273f, Enemy::Type::A },
        { 78.182f, Enemy::Type::B },
        { 78.636f, Enemy::Type::A },
        { 79.545f, Enemy::Type::A },
        { 80.455f, Enemy::Type::B },
        { 81.364f, Enemy::Type::A },
        { 82.273f, Enemy::Type::B },
        { 82.727f, Enemy::Type::A },
        { 83.636f, Enemy::Type::A, 0.909f },
        { 85.455f, Enemy::Type::B },
        { 86.364f, Enemy::Type::B },
        { 86.818f, Enemy::Type::A },
        { 87.727f, Enemy::Type::A },
        { 88.636f, Enemy::Type::B },
        { 89.545f, Enemy::Type::A },
        { 90.455f, Enemy::Type::B },
        { 90.909f, Enemy::Type::A },
        { 91.818f, Enemy::Type::A },
        { 92.727f, Enemy::Type::B },
        { 93.636f, Enemy::Type::A },
        { 94.545f, Enemy::Type::B, 0.4545f },
        { 95.000f, Enemy::Type::A },
        { 95.909f, Enemy::Type::A },
        { 96.818f, Enemy::Type::B },
        { 97.727f, Enemy::Type::A },
        { 98.636f, Enemy::Type::B },
        { 99.091f, Enemy::Type::A },
        { 100.000f, Enemy::Type::A },
        { 100.909f, Enemy::Type::B },
        { 101.818f, Enemy::Type::A },
        { 102.727f, Enemy::Type::B },
        { 103.182f, Enemy::Type::A },
        { 104.091f, Enemy::Type::A, 0.909f },
        { 105.909f, Enemy::Type::B },
        { 106.818f, Enemy::Type::B },
        { 107.273f, Enemy::Type::A },
        { 108.182f, Enemy::Type::A },
        { 109.091f, Enemy::Type::B },
        { 110.000f, Enemy::Type::A },
        { 110.909f, Enemy::Type::B },
        { 111.364f, Enemy::Type::A },
        { 112.273f, Enemy::Type::A },
        { 113.182f, Enemy::Type::B },
        { 114.091f, Enemy::Type::A },
        { 115.000f, Enemy::Type::B, 0.4545f },
        { 115.455f, Enemy::Type::A },
        { 116.364f, Enemy::Type::A },
        { 117.273f, Enemy::Type::B },
        { 118.182f, Enemy::Type::A },
        { 119.091f, Enemy::Type::B },
        { 119.545f, Enemy::Type::A },
        { 120.455f, Enemy::Type::A },
        { 121.364f, Enemy::Type::B },
        { 122.273f, Enemy::Type::A },
        { 123.182f, Enemy::Type::B },
        { 123.636f, Enemy::Type::A },
        { 124.545f, Enemy::Type::A, 0.909f },
        { 126.364f, Enemy::Type::B },
        { 127.273f, Enemy::Type::B },
        { 127.727f, Enemy::Type::A },
        { 128.636f, Enemy::Type::A },
        { 129.545f, Enemy::Type::B },
        { 130.455f, Enemy::Type::A },
        { 131.364f, Enemy::Type::B },
        { 131.818f, Enemy::Type::A },
        { 132.727f, Enemy::Type::A },
        { 133.636f, Enemy::Type::B },
        { 134.545f, Enemy::Type::A },
        { 135.455f, Enemy::Type::B, 0.4545f },
        { 135.909f, Enemy::Type::A },
        { 136.818f, Enemy::Type::A },
        { 137.727f, Enemy::Type::B },
        { 138.636f, Enemy::Type::A },
        { 139.545f, Enemy::Type::B },
        { 140.000f, Enemy::Type::A },
        { 140.909f, Enemy::Type::A },
        { 141.818f, Enemy::Type::B },
        { 142.727f, Enemy::Type::A },
        { 143.636f, Enemy::Type::B },
        { 144.091f, Enemy::Type::A },
        { 145.000f, Enemy::Type::A, 0.909f },
        { 146.818f, Enemy::Type::B },
        { 147.727f, Enemy::Type::B },
        { 148.182f, Enemy::Type::A },
        { 149.091f, Enemy::Type::A },
        { 150.000f, Enemy::Type::B },
        { 150.909f, Enemy::Type::A },
        { 151.818f, Enemy::Type::B },
        { 152.273f, Enemy::Type::A },
        { 153.182f, Enemy::Type::A },
        { 154.091f, Enemy::Type::B },
        { 155.000f, Enemy::Type::A },
        { 155.909f, Enemy::Type::B, 0.4545f },
        { 156.364f, Enemy::Type::A },
        { 157.273f, Enemy::Type::A },
        { 158.182f, Enemy::Type::B },
        { 159.091f, Enemy::Type::A },
        { 160.000f, Enemy::Type::B },
        { 160.455f, Enemy::Type::A },
        { 161.364f, Enemy::Type::A },
        { 162.273f, Enemy::Type::B },
        { 163.182f, Enemy::Type::A },
        { 164.091f, Enemy::Type::B },
        { 164.545f, Enemy::Type::A },
        { 165.455f, Enemy::Type::A, 0.909f },
        { 167.273f, Enemy::Type::B },
        { 168.182f, Enemy::Type::B },
        { 168.636f, Enemy::Type::A },
        { 169.545f, Enemy::Type::A },
        { 170.455f, Enemy::Type::B },
        { 171.364f, Enemy::Type::A },
    };
}

std::vector<BeatNote> getHard()
{
    return {
        { 0.909f, Enemy::Type::A },
        { 1.364f, Enemy::Type::A },
        { 1.818f, Enemy::Type::B },
        { 2.273f, Enemy::Type::A },
        { 2.727f, Enemy::Type::A },
        { 3.182f, Enemy::Type::A },
        { 3.636f, Enemy::Type::A },
        { 4.091f, Enemy::Type::A },
        { 4.545f, Enemy::Type::B },
        { 5.000f, Enemy::Type::A },
        { 5.455f, Enemy::Type::A },
        { 5.909f, Enemy::Type::A },
        { 6.364f, Enemy::Type::A },
        { 6.818f, Enemy::Type::A },
        { 7.273f, Enemy::Type::A },
        { 7.727f, Enemy::Type::A },
        { 8.182f, Enemy::Type::B, 0.4545f },
        { 9.091f, Enemy::Type::B },
        { 9.545f, Enemy::Type::B },
        { 10.000f, Enemy::Type::A },
        { 10.455f, Enemy::Type::A },
        { 10.909f, Enemy::Type::B },
        { 11.364f, Enemy::Type::B },
        { 11.818f, Enemy::Type::B },
        { 12.273f, Enemy::Type::A },
        { 12.727f, Enemy::Type::A },
        { 13.182f, Enemy::Type::B },
        { 13.636f, Enemy::Type::A },
        { 14.091f, Enemy::Type::A },
        { 14.545f, Enemy::Type::B },
        { 15.000f, Enemy::Type::A },
        { 15.455f, Enemy::Type::B },
        { 15.909f, Enemy::Type::B },
        { 16.364f, Enemy::Type::B },
        { 16.818f, Enemy::Type::A },
        { 17.273f, Enemy::Type::B, 0.4545f },
        { 18.182f, Enemy::Type::B },
        { 18.636f, Enemy::Type::A },
        { 19.091f, Enemy::Type::B },
        { 19.545f, Enemy::Type::B },
        { 20.000f, Enemy::Type::A },
        { 20.455f, Enemy::Type::A },
        { 20.909f, Enemy::Type::A },
        { 21.364f, Enemy::Type::A },
        { 21.818f, Enemy::Type::B },
        { 22.273f, Enemy::Type::A },
        { 22.727f, Enemy::Type::A },
        { 23.182f, Enemy::Type::A },
        { 23.636f, Enemy::Type::B },
        { 24.091f, Enemy::Type::B },
        { 24.545f, Enemy::Type::B },
        { 25.000f, Enemy::Type::B },
        { 25.455f, Enemy::Type::A },
        { 25.909f, Enemy::Type::B, 0.4545f },
        { 26.818f, Enemy::Type::B },
        { 27.273f, Enemy::Type::B },
        { 27.727f, Enemy::Type::A },
        { 28.182f, Enemy::Type::A },
        { 28.636f, Enemy::Type::A },
        { 29.091f, Enemy::Type::A },
        { 29.545f, Enemy::Type::B },
        { 30.000f, Enemy::Type::B },
        { 30.455f, Enemy::Type::B },
        { 30.909f, Enemy::Type::A },
        { 31.364f, Enemy::Type::B },
        { 31.818f, Enemy::Type::A },
        { 32.273f, Enemy::Type::A },
        { 32.727f, Enemy::Type::A },
        { 33.182f, Enemy::Type::B },
        { 33.636f, Enemy::Type::B },
        { 34.091f, Enemy::Type::B, 0.4545f },
        { 35.000f, Enemy::Type::A },
        { 35.455f, Enemy::Type::B },
        { 35.909f, Enemy::Type::A },
        { 36.364f, Enemy::Type::B },
        { 36.818f, Enemy::Type::B },
        { 37.273f, Enemy::Type::B },
        { 37.727f, Enemy::Type::A },
        { 38.182f, Enemy::Type::B, 0.4545f },
        { 38.636f, Enemy::Type::A },
        { 39.091f, Enemy::Type::A },
        { 39.545f, Enemy::Type::B },
        { 40.000f, Enemy::Type::B },
        { 40.455f, Enemy::Type::B },
        { 40.909f, Enemy::Type::B },
        { 41.364f, Enemy::Type::A },
        { 41.818f, Enemy::Type::A },
        { 42.273f, Enemy::Type::B, 0.4545f },
        { 43.182f, Enemy::Type::A },
        { 43.636f, Enemy::Type::A },
        { 44.091f, Enemy::Type::A },
        { 44.545f, Enemy::Type::A },
        { 45.000f, Enemy::Type::B },
        { 45.455f, Enemy::Type::A },
        { 45.909f, Enemy::Type::B },
        { 46.364f, Enemy::Type::B },
        { 46.818f, Enemy::Type::B },
        { 47.273f, Enemy::Type::B },
        { 47.727f, Enemy::Type::A },
        { 48.182f, Enemy::Type::A },
        { 48.636f, Enemy::Type::B },
        { 49.091f, Enemy::Type::B },
        { 49.545f, Enemy::Type::A },
        { 50.000f, Enemy::Type::B, 0.4545f },
        { 50.909f, Enemy::Type::B },
        { 51.364f, Enemy::Type::B },
        { 51.818f, Enemy::Type::A },
        { 52.273f, Enemy::Type::B },
        { 52.727f, Enemy::Type::A },
        { 53.182f, Enemy::Type::A },
        { 53.636f, Enemy::Type::B },
        { 54.091f, Enemy::Type::A },
        { 54.545f, Enemy::Type::A },
        { 55.000f, Enemy::Type::B },
        { 55.455f, Enemy::Type::A },
        { 55.909f, Enemy::Type::A },
        { 56.364f, Enemy::Type::B },
        { 56.818f, Enemy::Type::B },
        { 57.273f, Enemy::Type::A },
        { 57.727f, Enemy::Type::A, 0.4545f },
        { 58.636f, Enemy::Type::B },
        { 58.636f, Enemy::Type::B },
        { 59.091f, Enemy::Type::A },
        { 59.545f, Enemy::Type::A },
        { 60.000f, Enemy::Type::A },
        { 60.455f, Enemy::Type::A },
        { 60.909f, Enemy::Type::A },
        { 61.364f, Enemy::Type::B },
        { 61.818f, Enemy::Type::A },
        { 62.273f, Enemy::Type::A },
        { 62.727f, Enemy::Type::A },
        { 63.182f, Enemy::Type::B },
        { 63.636f, Enemy::Type::A },
        { 64.091f, Enemy::Type::B },
        { 64.545f, Enemy::Type::B },
        { 65.000f, Enemy::Type::A },
        { 65.455f, Enemy::Type::A, 0.4545f },
        { 66.364f, Enemy::Type::B },
        { 66.818f, Enemy::Type::B },
        { 67.273f, Enemy::Type::B },
        { 67.727f, Enemy::Type::B },
        { 68.182f, Enemy::Type::A },
        { 68.636f, Enemy::Type::A },
        { 69.091f, Enemy::Type::A },
        { 69.545f, Enemy::Type::A },
        { 70.000f, Enemy::Type::B },
        { 70.455f, Enemy::Type::A },
        { 70.909f, Enemy::Type::A },
        { 71.364f, Enemy::Type::A },
        { 71.818f, Enemy::Type::A },
        { 72.273f, Enemy::Type::A },
        { 72.727f, Enemy::Type::A },
        { 73.182f, Enemy::Type::A },
        { 73.636f, Enemy::Type::A },
        { 74.091f, Enemy::Type::A, 0.4545f },
        { 75.000f, Enemy::Type::B },
        { 75.455f, Enemy::Type::A },
        { 75.909f, Enemy::Type::B },
        { 76.364f, Enemy::Type::B },
        { 76.818f, Enemy::Type::A },
        { 77.273f, Enemy::Type::A },
        { 77.727f, Enemy::Type::B },
        { 78.182f, Enemy::Type::A },
        { 78.636f, Enemy::Type::B },
        { 79.091f, Enemy::Type::B },
        { 79.545f, Enemy::Type::A },
        { 80.000f, Enemy::Type::A },
        { 80.455f, Enemy::Type::A },
        { 80.909f, Enemy::Type::B },
        { 81.364f, Enemy::Type::B },
        { 81.818f, Enemy::Type::B },
        { 82.273f, Enemy::Type::B, 0.4545f },
        { 83.182f, Enemy::Type::B },
        { 83.636f, Enemy::Type::A },
        { 84.091f, Enemy::Type::A },
        { 84.545f, Enemy::Type::B },
        { 85.000f, Enemy::Type::B },
        { 85.455f, Enemy::Type::A },
        { 85.909f, Enemy::Type::A },
        { 86.364f, Enemy::Type::A },
        { 86.818f, Enemy::Type::A },
        { 87.273f, Enemy::Type::B },
        { 87.727f, Enemy::Type::A },
        { 88.182f, Enemy::Type::B },
        { 88.636f, Enemy::Type::A },
        { 89.091f, Enemy::Type::B },
        { 89.545f, Enemy::Type::B },
        { 90.000f, Enemy::Type::A },
        { 90.455f, Enemy::Type::A, 0.4545f },
        { 91.364f, Enemy::Type::B },
        { 91.818f, Enemy::Type::A },
        { 92.273f, Enemy::Type::A },
        { 92.727f, Enemy::Type::A },
        { 93.182f, Enemy::Type::A },
        { 93.636f, Enemy::Type::A },
        { 94.091f, Enemy::Type::B },
        { 94.545f, Enemy::Type::B },
        { 95.000f, Enemy::Type::B },
        { 95.455f, Enemy::Type::A },
        { 95.909f, Enemy::Type::B },
        { 96.364f, Enemy::Type::A },
        { 96.818f, Enemy::Type::A },
        { 97.273f, Enemy::Type::B },
        { 97.727f, Enemy::Type::A },
        { 98.182f, Enemy::Type::B },
        { 98.636f, Enemy::Type::B, 0.4545f },
        { 99.545f, Enemy::Type::B },
        { 99.545f, Enemy::Type::B },
        { 100.000f, Enemy::Type::B },
        { 100.455f, Enemy::Type::B },
        { 100.909f, Enemy::Type::A },
        { 101.364f, Enemy::Type::A },
        { 101.818f, Enemy::Type::B },
        { 102.273f, Enemy::Type::A },
        { 102.727f, Enemy::Type::A },
        { 103.182f, Enemy::Type::A },
        { 103.636f, Enemy::Type::B },
        { 104.091f, Enemy::Type::A },
        { 104.545f, Enemy::Type::A },
        { 105.000f, Enemy::Type::B },
        { 105.455f, Enemy::Type::A },
        { 105.909f, Enemy::Type::A, 0.4545f },
        { 106.818f, Enemy::Type::A },
        { 107.273f, Enemy::Type::A },
        { 107.727f, Enemy::Type::A },
        { 108.182f, Enemy::Type::A },
        { 108.636f, Enemy::Type::B },
        { 109.091f, Enemy::Type::A },
        { 109.545f, Enemy::Type::A },
        { 110.000f, Enemy::Type::A },
        { 110.455f, Enemy::Type::A },
        { 110.909f, Enemy::Type::B },
        { 111.364f, Enemy::Type::B },
        { 111.818f, Enemy::Type::B },
        { 112.273f, Enemy::Type::A },
        { 112.727f, Enemy::Type::B },
        { 113.182f, Enemy::Type::A },
        { 113.636f, Enemy::Type::B, 0.4545f },
        { 114.545f, Enemy::Type::B },
        { 115.000f, Enemy::Type::B },
        { 115.455f, Enemy::Type::B },
        { 115.909f, Enemy::Type::B },
        { 116.364f, Enemy::Type::A },
        { 116.818f, Enemy::Type::A },
        { 117.273f, Enemy::Type::B },
        { 117.727f, Enemy::Type::A },
        { 118.182f, Enemy::Type::A },
        { 118.636f, Enemy::Type::A },
        { 119.091f, Enemy::Type::B },
        { 119.545f, Enemy::Type::A },
        { 120.000f, Enemy::Type::B },
        { 120.455f, Enemy::Type::A },
        { 120.909f, Enemy::Type::A },
        { 121.364f, Enemy::Type::B },
        { 121.818f, Enemy::Type::B, 0.4545f },
        { 122.727f, Enemy::Type::A },
        { 122.727f, Enemy::Type::B },
        { 123.182f, Enemy::Type::B },
        { 123.636f, Enemy::Type::A },
        { 124.091f, Enemy::Type::B },
        { 124.545f, Enemy::Type::A },
        { 125.000f, Enemy::Type::A },
        { 125.455f, Enemy::Type::B },
        { 125.909f, Enemy::Type::A },
        { 126.364f, Enemy::Type::A },
        { 126.818f, Enemy::Type::A },
        { 127.273f, Enemy::Type::B },
        { 127.727f, Enemy::Type::B },
        { 128.182f, Enemy::Type::A },
        { 128.636f, Enemy::Type::B },
        { 129.091f, Enemy::Type::A },
        { 129.545f, Enemy::Type::B, 0.4545f },
        { 130.455f, Enemy::Type::B },
        { 130.909f, Enemy::Type::A },
        { 131.364f, Enemy::Type::A },
        { 131.818f, Enemy::Type::B },
        { 132.273f, Enemy::Type::B },
        { 132.727f, Enemy::Type::A },
        { 133.182f, Enemy::Type::A },
        { 133.636f, Enemy::Type::B },
        { 134.091f, Enemy::Type::A },
        { 134.545f, Enemy::Type::B },
        { 135.000f, Enemy::Type::A },
        { 135.455f, Enemy::Type::B },
        { 135.909f, Enemy::Type::B },
        { 136.364f, Enemy::Type::A },
        { 136.818f, Enemy::Type::A },
        { 137.273f, Enemy::Type::A, 0.4545f },
        { 138.182f, Enemy::Type::A },
        { 138.636f, Enemy::Type::B },
        { 139.091f, Enemy::Type::A },
        { 139.545f, Enemy::Type::B },
        { 140.000f, Enemy::Type::A },
        { 140.455f, Enemy::Type::A },
        { 140.909f, Enemy::Type::B },
        { 141.364f, Enemy::Type::B },
        { 141.818f, Enemy::Type::A },
        { 142.273f, Enemy::Type::B },
        { 142.727f, Enemy::Type::A },
        { 143.182f, Enemy::Type::A },
        { 143.636f, Enemy::Type::A },
        { 144.091f, Enemy::Type::B },
        { 144.545f, Enemy::Type::B },
        { 145.000f, Enemy::Type::A },
        { 145.455f, Enemy::Type::A, 0.4545f },
        { 146.364f, Enemy::Type::A },
        { 146.818f, Enemy::Type::B },
        { 147.273f, Enemy::Type::A },
        { 147.727f, Enemy::Type::A },
        { 148.182f, Enemy::Type::B },
        { 148.636f, Enemy::Type::B },
        { 149.091f, Enemy::Type::A },
        { 149.545f, Enemy::Type::B },
        { 150.000f, Enemy::Type::A },
        { 150.455f, Enemy::Type::A },
        { 150.909f, Enemy::Type::B },
        { 151.364f, Enemy::Type::A },
        { 151.818f, Enemy::Type::B },
        { 152.273f, Enemy::Type::A },
        { 152.727f, Enemy::Type::A },
        { 153.182f, Enemy::Type::B, 0.4545f },
        { 154.091f, Enemy::Type::B },
        { 154.545f, Enemy::Type::A },
        { 155.000f, Enemy::Type::A },
        { 155.455f, Enemy::Type::A },
        { 155.909f, Enemy::Type::A },
        { 156.364f, Enemy::Type::B },
        { 156.818f, Enemy::Type::B },
        { 157.273f, Enemy::Type::B },
        { 157.727f, Enemy::Type::A },
        { 158.182f, Enemy::Type::B },
        { 158.636f, Enemy::Type::B },
        { 159.091f, Enemy::Type::B },
        { 159.545f, Enemy::Type::B },
        { 160.000f, Enemy::Type::A },
        { 160.455f, Enemy::Type::A },
        { 160.909f, Enemy::Type::B, 0.4545f },
        { 161.818f, Enemy::Type::A },
        { 162.273f, Enemy::Type::A },
        { 162.727f, Enemy::Type::A },
        { 163.182f, Enemy::Type::B },
        { 163.636f, Enemy::Type::B },
        { 164.091f, Enemy::Type::B },
        { 164.545f, Enemy::Type::B },
        { 165.000f, Enemy::Type::A },
        { 165.455f, Enemy::Type::B },
        { 165.909f, Enemy::Type::A },
        { 166.364f, Enemy::Type::B },
        { 166.818f, Enemy::Type::A },
        { 167.273f, Enemy::Type::B },
        { 167.727f, Enemy::Type::A },
        { 168.182f, Enemy::Type::A },
        { 168.636f, Enemy::Type::B },
        { 169.091f, Enemy::Type::B, 0.4545f },
        { 170.000f, Enemy::Type::A },
        { 170.455f, Enemy::Type::B },
        { 170.909f, Enemy::Type::A },
        { 171.364f, Enemy::Type::B },
        { 171.818f, Enemy::Type::B },
        { 172.273f, Enemy::Type::B },
        { 172.727f, Enemy::Type::B },
        { 173.182f, Enemy::Type::B },
        { 173.636f, Enemy::Type::B },
    };
}

std::vector<BeatNote> getByDifficulty(Difficulty diff)
{
    switch (diff) {
        case Difficulty::Easy:   return getEasy();
        case Difficulty::Medium: return getMedium();
        case Difficulty::Hard:   return getHard();
    }
    return getEasy();
}

} // namespace Beatmaps

//  ENEMY MANAGER
EnemyManager::EnemyManager(sf::RenderWindow& window, float groundY)
: m_window(window)
, m_groundY(groundY)
{
    // Hit zone: area di sekitar posisi player (1/4 dari kiri layar).
    float winW       = float(m_window.getSize().x);
    float playerX    = winW * 0.25f;
    float zoneWidth  = 120.f;
    m_hitZoneCenter  = playerX * 2 - 80.f; // Sedikit di depan player supaya feel-nya lebih enak.
    m_hitZoneLeft    = m_hitZoneCenter - zoneWidth * 0.5f;
    m_hitZoneRight   = m_hitZoneCenter + zoneWidth * 0.5f;

    float size = 50.f;
    float y = m_groundY - size;

    m_targetCircle.setRadius(size / 2.f);
    m_targetCircle.setOrigin(size / 2.f, size / 2.f);
    m_targetCircle.setPosition(m_hitZoneCenter, y);
    m_targetCircle.setFillColor(sf::Color(0, 220, 255, 90));
    m_targetCircle.setOutlineColor(sf::Color(140, 255, 255, 200));
    m_targetCircle.setOutlineThickness(3.f);
    m_targetCircle.setPointCount(40);

    m_targetBox.setSize({size, size});
    m_targetBox.setOrigin(size / 2.f, size / 2.f);
    m_targetBox.setPosition(m_hitZoneCenter, y - (size * 2.5f + 40.f));
    m_targetBox.setFillColor(sf::Color(255, 0, 200, 90));
    m_targetBox.setOutlineColor(sf::Color(255, 140, 255, 200));
    m_targetBox.setOutlineThickness(3.f);

    m_beatmap = Beatmaps::getByDifficulty(Difficulty::Easy);
}

void EnemyManager::setDifficulty(Difficulty diff)
{
    m_beatmap = Beatmaps::getByDifficulty(diff);
    m_nextNote = 0;
}

void EnemyManager::update(float dt)
{
    m_stageTimer += dt;

    while (m_nextNote < int(m_beatmap.size()) &&
           m_stageTimer >= m_beatmap[m_nextNote].spawnTime)
    {
        float travelDistance = spawnX() - m_hitZoneCenter;
        float travelTime     = travelDistance / m_defaultSpeed;
        float targetHitTime  = m_beatmap[m_nextNote].spawnTime + travelTime;

        auto enemy = std::make_unique<Enemy>(
            m_window,
            m_beatmap[m_nextNote].type,
            spawnX(),
            m_groundY,
            targetHitTime,
            m_beatmap[m_nextNote].holdDuration
        );

        enemy->setSpeed(m_defaultSpeed);
        enemy->setColor(
            m_beatmap[m_nextNote].type == Enemy::Type::A ? m_defaultColorA : m_defaultColorB
        );

        m_enemies.push_back(std::move(enemy));
        m_nextNote++;
    }

    for (auto& e : m_enemies) {
        e->update(dt);
    }

    // Bersihkan referensi hold aktif kalau musuhnya sudah mati/passed,
    // supaya tidak ada dangling pointer.
    if (m_activeHoldA && (m_activeHoldA->isDead() || m_activeHoldA->isPassed()))
        m_activeHoldA = nullptr;
    if (m_activeHoldB && (m_activeHoldB->isDead() || m_activeHoldB->isPassed()))
        m_activeHoldB = nullptr;

    m_enemies.erase(
        std::remove_if(m_enemies.begin(), m_enemies.end(),
            [](const std::unique_ptr<Enemy>& e) { return e->isDead() || e->isPassed(); }),
        m_enemies.end()
    );
}

void EnemyManager::draw()
{
    for (auto& e : m_enemies) {
        e->draw();
    }
}
void EnemyManager::drawTargets()
{
    m_window.draw(m_targetCircle);
    m_window.draw(m_targetBox);
}
Judgement EnemyManager::judgeTiming(float deltaSeconds) const
{
    float d = std::abs(deltaSeconds);
    if (d <= kPerfectWindow) return Judgement::Perfect;
    if (d <= kGoodWindow)    return Judgement::Good;
    if (d <= kBadWindow)     return Judgement::Bad;
    return Judgement::Miss;
}
// Gabungkan judgement awal (timing tekan) dengan rasio hold (0..1).
// Hasil akhir = yang lebih jelek di antara keduanya, supaya tidak bisa
// "curang" pencet ngasal lalu tahan penuh dan tetap dapat Perfect.
Judgement EnemyManager::combineJudgement(Judgement startJ, float holdRatio) const
{
    Judgement holdJ;
    if (holdRatio >= 0.95f)      holdJ = Judgement::Perfect;
    else if (holdRatio >= 0.75f) holdJ = Judgement::Good;
    else if (holdRatio >= 0.40f) holdJ = Judgement::Bad;
    else                          holdJ = Judgement::Miss;

    // Urutan dari terbaik ke terjelek, ambil yang index-nya lebih besar (lebih jelek)
    auto rank = [](Judgement j) {
        switch (j) {
            case Judgement::Perfect: return 0;
            case Judgement::Good:    return 1;
            case Judgement::Bad:     return 2;
            case Judgement::Miss:    return 3;
            default:                  return 3;
        }
    };

    return (rank(startJ) >= rank(holdJ)) ? startJ : holdJ;
}

HitResult EnemyManager::resolveHit(Enemy::Type expectedType, Enemy** activeHoldSlot)
{
    HitResult result;

    Enemy* target = nullptr;
    float  bestLeft = 1e9f;

    for (auto& e : m_enemies) {
        if (e->isDead()) continue;
        if (e->isHoldNote() && e->getHoldState() != Enemy::HoldState::Idle) continue; // sudah diproses

        float cx = e->getBounds().left + e->getBounds().width / 2.f;
        if (cx >= m_hitZoneLeft && cx <= m_hitZoneRight) {
            float left = e->getBounds().left;
            if (left < bestLeft) {
                bestLeft = left;
                target   = e.get();
            }
        }
    }

    if (!target) {
        result.judgement   = Judgement::None;
        result.correctType = false;
        result.hasPosition = false;
        return result;
    }

    float delta = m_stageTimer - target->getTargetHitTime();
    Judgement startJ = judgeTiming(delta);
    bool typeCorrect = (target->getType() == expectedType);

    sf::FloatRect b = target->getBounds();
    result.position    = { b.left + b.width / 2.f, b.top };
    result.hasPosition  = true;
    result.isHoldNote   = target->isHoldNote();

    if (!typeCorrect) {
        // Jenis salah: kalau ini hold note, batalkan saja (tidak masuk active hold)
        result.judgement   = Judgement::Miss;
        result.correctType = false;
        target->kill();
        return result;
    }

    if (!target->isHoldNote()) {
        // Tap note biasa: langsung final
        result.judgement = startJ;
        target->kill();
        return result;
    }

    // Hold note: mulai menahan, judgement BELUM final.
    // Simpan startJ sementara tidak dikembalikan penuh — kita kembalikan
    // judgement starting saja sebagai preview, caller (Stage/HUD) bisa
    // memilih untuk tidak menampilkan apapun sampai release/selesai kalau mau.
    target->startHold(m_stageTimer);
    *activeHoldSlot = target;

    result.judgement = startJ;   // preview sementara, BUKAN final
    return result;
}

HitResult EnemyManager::onPunch()
{
    return resolveHit(Enemy::Type::A, &m_activeHoldA);
}

HitResult EnemyManager::onKick()
{
    return resolveHit(Enemy::Type::B, &m_activeHoldB);
}

// ── Hit resolution untuk RELEASE (lepas tombol sebelum hold selesai) ──
HitResult EnemyManager::resolveRelease(Enemy::Type /*expectedType*/, Enemy** activeHoldSlot)
{
    HitResult result;

    Enemy* target = *activeHoldSlot;
    if (!target || target->getHoldState() != Enemy::HoldState::Holding) {
        result.judgement = Judgement::None;
        return result;
    }

    // Hitung judgement awal dari targetHitTime vs waktu mulai hold tadi
    // (kita tidak simpan startJ secara eksplisit, jadi hitung ulang dari
    // selisih waktu mulai hold terhadap targetHitTime — cukup akurat).
    float startDelta = (target->getTargetHitTime() >= 0.f)
        ? 0.f  // placeholder, startJ asli sudah dipakai di resolveHit; di sini
               // kita fokus ke kualitas hold-nya karena start sudah lewat threshold ok/tidak.
        : 0.f;
    (void)startDelta;

    float holdRatio = target->releaseHold();   // 0..1, seberapa jauh hold sempat berjalan

    // Karena release terjadi SEBELUM holdDuration selesai, hold dianggap gagal
    // sebagian — judgement ditentukan murni dari holdRatio (dilepas dini = jelek).
    Judgement j;
    if (holdRatio >= 0.95f)      j = Judgement::Perfect;
    else if (holdRatio >= 0.75f) j = Judgement::Good;
    else if (holdRatio >= 0.40f) j = Judgement::Bad;
    else                          j = Judgement::Miss;

    sf::FloatRect b = target->getBounds();
    result.judgement   = j;
    result.correctType = true;
    result.position     = { b.left + b.width / 2.f, b.top };
    result.hasPosition   = true;
    result.isHoldNote    = true;

    target->kill();
    *activeHoldSlot = nullptr;
    return result;
}

HitResult EnemyManager::onPunchRelease()
{
    return resolveRelease(Enemy::Type::A, &m_activeHoldA);
}

HitResult EnemyManager::onKickRelease()
{
    return resolveRelease(Enemy::Type::B, &m_activeHoldB);
}

// ── Update hold note tiap frame (auto-selesai kalau durasi tercapai) ──
void EnemyManager::updateHolds(float dt, bool isPunchHeld, bool isKickHeld,
                                 HitResult& outPunchResult, HitResult& outKickResult)
{
    outPunchResult = HitResult{};
    outKickResult  = HitResult{};
    if (m_activeHoldA) {
        if (!isPunchHeld) {
            // Tombol dilepas di luar jalur normal (misal player lepas tanpa
            // event terdeteksi tepat) -> selesaikan sebagai release dini.
            outPunchResult = resolveRelease(Enemy::Type::A, &m_activeHoldA);
        } else {
            bool finished = m_activeHoldA->updateHold(dt, m_stageTimer);
            if (finished) {
                sf::FloatRect b = m_activeHoldA->getBounds();
                outPunchResult.judgement   = Judgement::Perfect; // full hold = Perfect
                outPunchResult.correctType = true;
                outPunchResult.position     = { b.left + b.width / 2.f, b.top };
                outPunchResult.hasPosition   = true;
                outPunchResult.isHoldNote    = true;

                m_activeHoldA->kill();
                m_activeHoldA = nullptr;
            }
        }
    }
    if (m_activeHoldB) {
        if (!isKickHeld) {
            outKickResult = resolveRelease(Enemy::Type::B, &m_activeHoldB);
        } else {
            bool finished = m_activeHoldB->updateHold(dt, m_stageTimer);
            if (finished) {
                sf::FloatRect b = m_activeHoldB->getBounds();
                outKickResult.judgement   = Judgement::Perfect;
                outKickResult.correctType = true;
                outKickResult.position     = { b.left + b.width / 2.f, b.top };
                outKickResult.hasPosition   = true;
                outKickResult.isHoldNote    = true;

                m_activeHoldB->kill();
                m_activeHoldB = nullptr;
            }
        }
    }
}

void EnemyManager::reset()
{
    m_enemies.clear();
    m_stageTimer = 0.f;
    m_nextNote   = 0;
    m_activeHoldA = nullptr;
    m_activeHoldB = nullptr;
}