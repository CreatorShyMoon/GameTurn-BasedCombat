#include <iostream>   // Для ввода/вывода (cin, cout)
#include <string>     // Для использования std::string
#include <vector>     // Для динамических массивов std::vector
#include <memory>     // Для умных указателей std::shared_ptr
#include <cstdlib>    // Для rand(), srand()
#include <ctime>      // Для time() — инициализация генератора случайных чисел

using namespace std;  // Чтобы не писать std:: постоянно

//  ИНТЕРФЕЙС ДЕЙСТВИЯ (Стратегия) 
// Паттерн "Стратегия" позволяет менять поведение объекта во время выполнения.
// В моем случае "стратегия" — это действие: атака, блок, огненный шар и т.д.
class Entity; // Предварительное объявление, чтобы IAction мог ссылаться на Entity

class IAction {
public:
	// Основной метод, который выполняет действие
	virtual void Execute(Entity* actor, Entity* target) = 0;

	// Метод для получения имени действия
	virtual string Name() const = 0;

	// Виртуальный деструктор — чтобы корректно удалялись наследники
	virtual ~IAction() {}
};

// КЛАСС СУЩНОСТИ 
// Базовый класс для всех персонажей: игрока, гоблина и т.д.
class Entity {
protected:
	string name;  // Имя персонажа
	int health;   // Здоровье
	int energy;   // Энергия (не используется, но можно расширить)
	int mana;     // Мана для заклинаний
	int armor;    // Броня, уменьшает урон
	bool isBlocking;  // Флаг блокирования
	vector< shared_ptr<IAction> > actions; // Список действий (стратегий)
	/*shared_ptr — это умный указатель, который автоматически управляет памятью объекта в C++.
	Что делает: Когда объект больше не нужен (на него больше никто не ссылается), 
	shared_ptr автоматически удаляет его из памяти.
*/
public:
	// Конструктор с инициализацией всех полей
	Entity(string n, int h, int e, int m, int a)
		: name(n), health(h), energy(e), mana(m), armor(a), isBlocking(false) {}

	virtual ~Entity() {} // Виртуальный деструктор

	// Добавление действия к сущности
	void AddAction(shared_ptr<IAction> act) { actions.push_back(act); }

	// Проверка, хватает ли маны
	bool HasMana(int amount) const { return mana >= amount; }

	// Потратить ману
	void SpendMana(int amount) {
		mana -= amount;
		if (mana < 0) mana = 0; // Мана не может быть меньше 0
	}

void PerformAction(int index, Entity* target) {
    if (index >= 0 && (size_t)index < actions.size())
        actions[(size_t)index]->Execute(this, target);
    else
        cout << name << " не знает такого действия!\n";
}

	// Получение урона
	void TakeDamage(int dmg) {
		int real = dmg - armor; // Снижение урона за счет брони
		if (real < 0) real = 0;

		if (isBlocking) {   // Если персонаж встал в блок
			real /= 2;      // Урон уменьшается вдвое
			cout << name << " блокирует часть урона!\n";
			isBlocking = false; // После блока блокировка снимается
		}

		health -= real;
		if (health < 0) health = 0;

		cout << name << " получил " << real << " урона. [HP: " << health << "]\n";
	}

	bool IsAlive() const { return health > 0; } // Проверка жив ли персонаж
	string GetName() const { return name; }
	int GetMana() const { return mana; }
	void SetBlocking(bool val) { isBlocking = val; }

	// Восстановление здоровья
	void Heal(int amount) {
		health += amount;
		if (health > 100) health = 100;
		cout << name << " восстановил здоровье до " << health << endl;
	}

	// Восстановление маны
	void RestoreMana(int amount) {
		mana += amount;
		if (mana > 40) mana = 40;
		cout << name << " восстановил ману до " << mana << endl;
	}
};

// КОНКРЕТНЫЕ ДЕЙСТВИЯ 

// Атака (простое физическое действие)
class Attack : public IAction {
public:
	void Execute(Entity* actor, Entity* target) {
		cout << actor->GetName() << " атакует " << target->GetName() << "!\n";
		target->TakeDamage(15);
	}
	string Name() const { return "Атака"; }
};

// Огненный шар (заклинание, тратит ману)
class Fireball : public IAction {
public:
	void Execute(Entity* actor, Entity* target) {
		const int manaCost = 10;
		if (!actor->HasMana(manaCost)) {
			cout << actor->GetName() << " пытается использовать огненный шар, но не хватает маны!\n";
			return;
		}
		actor->SpendMana(manaCost);
		cout << actor->GetName() << " бросает огненный шар в " << target->GetName() 
			 << "! (-" << manaCost << " маны)\n";
		target->TakeDamage(25);
	}
	string Name() const { return "Огненный шар"; }
};

// Блок (снижает урон на половину)
class Block : public IAction {
public:
	void Execute(Entity* actor, Entity* target) {
		(void)target; // Параметр не используется
		cout << actor->GetName() << " встал в блок!\n";
		actor->SetBlocking(true);
	}
	string Name() const { return "Блок"; }
};

// ФАБРИКА ДЕЙСТВИЙ 
// Паттерн "Фабрика" позволяет создавать объекты без указания конкретного типа.
// В нашем случае фабрика создает действия (Attack, Fireball, Block)
// Это удобно, чтобы не писать new Attack() внутри сущности — легче расширять код.
class ActionFactory {
public:
	static shared_ptr<IAction> CreateAction(const string& actionName) {
		if (actionName == "Attack") return shared_ptr<IAction>(new Attack());
		if (actionName == "Fireball") return shared_ptr<IAction>(new Fireball());
		if (actionName == "Block") return shared_ptr<IAction>(new Block());
		return shared_ptr<IAction>(); // Возвращает пустой указатель, если действие не найдено
	}
};

//  СУЩНОСТИ ИГРОКА И ГОБЛИНА 
class Player : public Entity {
public:
	Player(string n) : Entity(n, 100, 50, 40, 5) {
		// Добавляем действия через фабрику
		AddAction(ActionFactory::CreateAction("Attack"));
		AddAction(ActionFactory::CreateAction("Fireball"));
		AddAction(ActionFactory::CreateAction("Block"));
	}
};

class Goblin : public Entity {
public:
	Goblin(string n) : Entity(n, 80, 40, 0, 7) {
		AddAction(ActionFactory::CreateAction("Attack"));
		AddAction(ActionFactory::CreateAction("Block"));
	}
};

// ФУНКЦИЯ БОЯ 
bool StepBattle(Player& p, Goblin& g) {
	cout << "\n БОЙ НАЧАЛСЯ \n";

	while (p.IsAlive() && g.IsAlive()) {
		// Ход игрока
		cout << "\n ХОД ГЕРОЯ \n";
		cout << "Выберите действие:\n1. Атака\n2. Огненный шар\n3. Блок\n>>> ";
		int choice; 
		cin >> choice;
		p.PerformAction(choice-1, &g); // Используем стратегию (выбор действия)

		if (!g.IsAlive()) break;

		// Ход гоблина
		cout << "\n---- ХОД ГОБЛИНА ----\n";
		int gobChoice = rand() % 2; // Случайно выбираем действие
		g.PerformAction(gobChoice, &p);
		if (!p.IsAlive()) break;
	}

	cout << "\n БОЙ ОКОНЧЕН \n";
	if (!p.IsAlive()) { cout << "Герой погиб...\n"; return false; }
	cout << "Гоблин побеждён!\n";
	return true;
}

// ЗДАРОВА КАРТА
class GameMap {
public:
	static const int W = 15, H = 15;
	vector<vector<int> > grid; // Матрица карты
	size_t px, py; // Позиция игрока
	size_t gx, gy; // Позиция гоблина

	GameMap() {
		grid = vector<vector<int> >(H, vector<int>(W, 0)); // Инициализация карты нулями

		// Ставим стены по краям
		for (size_t i=0;i<W;i++){ grid[0][i]=9; grid[H-1][i]=9; }
		for (size_t i=0;i<H;i++){ grid[i][0]=9; grid[i][W-1]=9; }

		px=5; py=5; grid[py][px]=1; // Игрок
		gx=10; gy=10; grid[gy][gx]=2; // Гоблин

		// Прочие элементы карты
		grid[6][4]=3; 
		gx=11; gy=13; grid[gy][gx]=2;
		grid[7][10]=4; 
		grid[3][5]=5;
		grid[6][4]=3; 
		grid[8][4]=4; 
		grid[9][2]=5;
		grid[4][7]=3; 
		grid[10][3]=4; 
		grid[9][4]=5;
	}

	// Вывод карты на экран
	void Print() {
		for (size_t y = 0; y < H; y++) {
			for (size_t x = 0; x < W; x++) {
				cout << grid[y][x] << " ";
			}
			cout << endl;
		}
	}

	// Движение игрока
	bool MovePlayer(char cmd, Player& p, Goblin& g) {
		size_t nx=px, ny=py;
		if(cmd=='w') ny--; else if(cmd=='s') ny++;
		else if(cmd=='a') nx--; else if(cmd=='d') nx++;
		else { cout << "Неизвестная команда.\n"; return true; }

		if(grid[ny][nx]==9){ cout<<"Там стена!\n"; return true; }
		if(grid[ny][nx]==2){ 
			cout<<"Ты встретил гоблина!\n"; 
			if(!StepBattle(p,g)) return false; 
		}
		if(grid[ny][nx]==3){ cout<<"Хилка! +20 HP\n"; p.Heal(20); }
		if(grid[ny][nx]==4){ cout<<"Мана! +20 Mana\n"; p.RestoreMana(20); }
		if(grid[ny][nx]==5){ cout<<"Ты нашёл золото!\n"; }

		grid[py][px]=0; px=nx; py=ny; grid[py][px]=1;
		return true;
	}
};


int main() {
	srand((unsigned int)time(NULL)); // Инициализация генератора случайных чисел

	Player player("Герой");   // Создаем игрока
	Goblin goblin("Гоблин");  // Создаем гоблина
	GameMap map;               // Создаем карту

	cout << "Управление: W A S D\n";

	while(true){
		map.Print();
		cout << "Ход героя >>> ";
		char c; 
		cin >> c;
		if(!map.MovePlayer(c, player, goblin)) break; // Двигаем игрока
		cout << endl;
	}

	cout << "Игра завершена.\n";
	return 0;
}

/*
Комментарии по паттернам:

1. Стратегия (Strategy):
- IAction — интерфейс для всех действий персонажей.
- Каждое действие (Attack, Fireball, Block) — это конкретная стратегия.
- Персонаж хранит список стратегий и выбирает одну во время боя.
- Преимущество: можно менять поведение персонажа динамически, добавлять новые действия без изменения кода сущности.

2. Фабрика (Factory):
- ActionFactory создает объекты действий по имени.
- Персонаж не знает конкретный класс действия - только просит фабрику создать его.
- Преимущество: легкое расширение (новые действия), меньше зависимостей, код сущности не меняется.
*/
