#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>
#include <fstream>
#include <any>
#include <sqlite3.h>
#include <stdexcept>


enum class Priority {Low, Medium, High};
enum class Status {Open, InProgress, Done};

class Task {
	private:
		std::string title;
		std::string category;
		std::string dueDate;
		Priority priority;
		Status status;

	public:
		Task(std::string title, std::string category, std::string dueDate, Priority priority, Status status)
			: title(std::move(title)), category(std::move(category)), dueDate(std::move(dueDate)),
			priority(priority), status(status)
		{}

		const std::string& getTitle() const { return title; }
		const std::string& getCategory() const { return category; }
		const std::string& getDueDate() const { return dueDate; }
		Priority getPriority() const { return priority; }
		Status getStatus() const { return status; }

		void setPriority(Priority prio) { priority = prio; }
		void setStatus(Status stat) { status = stat; }

		void print() const {
			std::cout << "Title: " << title << ", Category: " << category << ", Due Date: " << dueDate;
			switch(priority) {
				case Priority::Low: std::cout << ", Priority: \033[32mLow\033[0m"; break;
				case Priority::Medium: std::cout << ", Priority: \033[33mMedium\033[0m"; break;
				case Priority::High: std::cout << ", Priority: \033[31mHigh\033[0m"; break;
			}		
			switch(status) {
				case Status::Open: std::cout << ", Status: Open" << std::endl; break;
				case Status::InProgress: std::cout << ", Status: In Progress" << std::endl; break;
				case Status::Done: std::cout << ", Status: Done" << std::endl; break;
			}		
		}
};

class TaskManager {
	private:
		sqlite3* db;

	public:
		TaskManager() {
			if (sqlite3_open("./data/tasks_sql.db", &db) != SQLITE_OK) {
				std::string errorMsg = sqlite3_errmsg(db);
				sqlite3_close(db);
				throw std::runtime_error("Failed to open database: " + errorMsg);
			}

			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				CREATE TABLE IF NOT EXISTS tasks (
					title 		TEXT		PRIMARY KEY,
					category	TEXT		NOT NULL,
					dueDate		TEXT		NOT NULL,
					priority	INTEGER		NOT NULL,
					status		INTEGER		NOT NULL
					);
				)", -1, &stmt, nullptr);
				
			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		};

		~TaskManager() {
			sqlite3_close(db);
		}


		std::vector<Task> getAllTasks() const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks;
				)", -1, &stmt, nullptr);

			std::vector<Task> allTasks;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string title 	 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				std::string dueDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
				Priority priority 	 = static_cast<Priority>(sqlite3_column_int(stmt, 4));
				Status status 		 = static_cast<Status>(sqlite3_column_int(stmt, 5));
				allTasks.push_back(Task(title, category, dueDate, priority, status));
			}

			sqlite3_finalize(stmt);

			return allTasks;
		}

		std::vector<std::string> getAvailableCategories() const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks;
				)", -1, &stmt, nullptr);

			std::vector<std::string> availableCategories;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				if (std::find(availableCategories.begin(), availableCategories.end(), category) == availableCategories.end()) {
					availableCategories.push_back(category);
				}
			}

			sqlite3_finalize(stmt);

			return availableCategories;
		}

		std::vector<Priority> getAvailablePriorities() const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks;
				)", -1, &stmt, nullptr);

			std::vector<Priority> availablePriorities;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				Priority priority = static_cast<Priority>(sqlite3_column_int(stmt, 4));
				if (std::find(availablePriorities.begin(), availablePriorities.end(), priority) == availablePriorities.end()) {
					availablePriorities.push_back(priority);
				}
			}

			sqlite3_finalize(stmt);

			return availablePriorities;
		}

		std::vector<Status> getAvailableStatuses() const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks;
				)", -1, &stmt, nullptr);

			std::vector<Status> availableStatuses;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				Status status = static_cast<Status>(sqlite3_column_int(stmt, 5));
				if (std::find(availableStatuses.begin(), availableStatuses.end(), status) == availableStatuses.end()) {
					availableStatuses.push_back(status);
				}
			}

			sqlite3_finalize(stmt);

			return availableStatuses;
		}


		bool addTask(const Task& task) {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				INSERT INTO tasks (title, category, dueDate, priority, status) VALUES (?, ?, ?, ?, ?);
				)", -1, &stmt, nullptr);

			sqlite3_bind_text(stmt, 1, task.getTitle().c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 2, task.getCategory().c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_text(stmt, 3, task.getDueDate().c_str(), -1, SQLITE_STATIC);
			sqlite3_bind_int(stmt, 4, static_cast<int>(task.getPriority()));
			sqlite3_bind_int(stmt, 5, static_cast<int>(task.getStatus()));

			int result = sqlite3_step(stmt);
			sqlite3_finalize(stmt);

			if (result == SQLITE_CONSTRAINT) {
				std::cout << "\033[31mTask '" << task.getTitle() << "' already exists.\033[0m" << std::endl;
				return false;
			}
			return result == SQLITE_DONE;
		}

		bool removeTask(const std::string& title) {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				DELETE FROM tasks WHERE title = ?;
				)", -1, &stmt, nullptr);

			sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);

			int result = sqlite3_step(stmt);
			sqlite3_finalize(stmt);

			return result == SQLITE_DONE && sqlite3_changes(db) > 0;
		}


		std::optional<Task> findTask(const std::string& title) const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks WHERE title = ?;
				)", -1, &stmt, nullptr);

			sqlite3_bind_text(stmt, 1, title.c_str(), -1, SQLITE_STATIC);

			std::optional<Task> foundTask = std::nullopt;
			if (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string title 	 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				std::string dueDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
				Priority priority 	 = static_cast<Priority>(sqlite3_column_int(stmt, 4));
				Status status 		 = static_cast<Status>(sqlite3_column_int(stmt, 5));
				foundTask = Task(title, category, dueDate, priority, status);
			};

			sqlite3_finalize(stmt);

			return foundTask;
		}


		std::vector<Task> filterByCategory(const std::string& cat) const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks WHERE category = ?;
				)", -1, &stmt, nullptr);

			sqlite3_bind_text(stmt, 1, cat.c_str(), -1, SQLITE_STATIC);

			std::vector<Task> filteredCategoryTasks;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string title 	 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				std::string dueDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
				Priority priority 	 = static_cast<Priority>(sqlite3_column_int(stmt, 4));
				Status status 		 = static_cast<Status>(sqlite3_column_int(stmt, 5));
				filteredCategoryTasks.push_back(Task(title, category, dueDate, priority, status));
			}

			sqlite3_finalize(stmt);

			return filteredCategoryTasks;
		}

		std::vector<Task> filterByPriority(Priority prio) const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks WHERE priority = ?;
				)", -1, &stmt, nullptr);

			sqlite3_bind_int(stmt, 1, static_cast<int>(prio));

			std::vector<Task> filteredPriorityTasks;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string title 	 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				std::string dueDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
				Priority priority 	 = static_cast<Priority>(sqlite3_column_int(stmt, 4));
				Status status 		 = static_cast<Status>(sqlite3_column_int(stmt, 5));
				filteredPriorityTasks.push_back(Task(title, category, dueDate, priority, status));
			}

			sqlite3_finalize(stmt);

			return filteredPriorityTasks;
		}

		std::vector<Task> filterByStatus(Status stat) const {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks WHERE status = ?;
				)", -1, &stmt, nullptr);

			sqlite3_bind_int(stmt, 1, static_cast<int>(stat));

			std::vector<Task> filteredStatusTasks;
			while (sqlite3_step(stmt) == SQLITE_ROW) {
				std::string title 	 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
				std::string category = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
				std::string dueDate  = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
				Priority priority 	 = static_cast<Priority>(sqlite3_column_int(stmt, 4));
				Status status 		 = static_cast<Status>(sqlite3_column_int(stmt, 5));
				filteredStatusTasks.push_back(Task(title, category, dueDate, priority, status));
			}

			sqlite3_finalize(stmt);

			return filteredStatusTasks;
		}


		void sortByTitle() {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks GROUP BY title;
				)", -1, &stmt, nullptr);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}

		void sortByCategory() {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks GROUP BY category;
				)", -1, &stmt, nullptr);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}

		void sortByPriority() {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks GROUP BY priority;
				)", -1, &stmt, nullptr);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}

		void sortByStatus() {
			sqlite3_stmt* stmt;
			sqlite3_prepare_v2(db, R"(
				SELECT * FROM tasks GROUP BY status;
				)", -1, &stmt, nullptr);

			sqlite3_step(stmt);
			sqlite3_finalize(stmt);
		}
};


Priority strToPrio(const std::string& inp) {
	std::string lower = inp;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	if (lower == "low") {
		return Priority::Low;
	}
	if (lower == "medium") {
		return Priority::Medium;
	}
	if (lower == "high") {
		return Priority::High;
	}
	throw std::invalid_argument("\033[31mInvalid Priority:\033[0m " + inp);
}

Status strToStat(const std::string& inp) {
	std::string lower = inp;
	std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
	if (lower == "open") {
		return Status::Open;
	}
	if (lower == "inprogress" || lower == "in progress") {
		return Status::InProgress;
	}
	if (lower == "done") {
		return Status::Done;
	}
	throw std::invalid_argument("\033[31mInvalid Status:\033[0m " + inp);
}

std::string PrioToStr (const Priority& prio) {
	if (prio == Priority::Low) {
		return "Low";
	}
	if (prio == Priority::Medium) {
		return "Medium";
	}
	if (prio == Priority::High) {
		return "High";
	}
	throw std::invalid_argument("\033[31mInvalid Priority.\033[0m");
}

std::string StatToStr (const Status& stat) {
	if (stat == Status::Open) {
		return "Open";
	}
	if (stat == Status::InProgress) {
		return "InProgress";
	}
	if (stat == Status::Done) {
		return "Done";
	}
	throw std::invalid_argument("\033[31mInvalid Status.\033[0m");
}


constexpr const char* EXIT_STR = "0";

std::string valiDATE() {
	const int MIN = 1;
    const int MONTHS = 12;
    const int LONG_MONTH_DAYS = 31;
    const int SHORT_MONTH_DAYS = 30;
    const int FEBRUARY_DAYS = 28;
    const int LEAP_FEBRUARY_DAYS = 29;

    const std::vector<int> LONG_MONTHS = {1,3,5,7,8,10,12};
    const std::vector<int> SHORT_MONTHS = {4,6,9,11};
    const int FEBRUARY_MONTH = 2;

    const int DATE_LENGTH = 10;

    const std::vector<char> SEPARATORS = {'-', '/', '.', ','};

    std::string date;
    bool date_valid = false;
    int day, month, year;
    int max_days = 0;

    do {
		std::cout << "-> ";
		std::getline(std::cin, date);

		if (date == EXIT_STR) {
			return date;
			break;
		}

        if (date.size() != DATE_LENGTH) {
            std::cout << "\033[31mInvalid date format.\033[0m" << std::endl;
            continue;
        }
        char sep1 = date[2];
        char sep2 = date[5];

        if ((std::find(SEPARATORS.begin(), SEPARATORS.end(), sep1) == SEPARATORS.end()) || 
        (std::find(SEPARATORS.begin(), SEPARATORS.end(), sep2) == SEPARATORS.end())) {
            std::cout << "\033[31mInvalid date format.\033[0m" << std::endl;
            continue;
        }
        try {
            day = std::stoi(date.substr(0,2));
            month = std::stoi(date.substr(3,2));
            year = std::stoi(date.substr(6,4));
        }
        catch (const std::exception&) {
            std::cout << "\033[31mInvalid date format.\033[0m" << std::endl;
            continue;
        }
        if (day >= MIN && day <= LONG_MONTH_DAYS && month >= MIN && month <= MONTHS && year >= MIN) {
            // Check february
            if (month == FEBRUARY_MONTH) {
                if ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) {
                    max_days = LEAP_FEBRUARY_DAYS;
                }
                else {
                    max_days = FEBRUARY_DAYS;
                }
            }
            // Check long months
            else if (std::find(LONG_MONTHS.begin(), LONG_MONTHS.end(), month) != LONG_MONTHS.end()) {
                max_days = LONG_MONTH_DAYS;
            }
            // Check short months
            else if (std::find(SHORT_MONTHS.begin(), SHORT_MONTHS.end(), month) != SHORT_MONTHS.end()) {
                max_days = SHORT_MONTH_DAYS;
            }
            if (day <= max_days) {
                date_valid = true;
            }
            else {
                std::cout << "\033[31mInvalid date.\033[0m" << std::endl;
            }     
        }
        else {
            std::cout << "\033[31mInvalid date.\033[0m" << std::endl;
        } 
    } while (date_valid == false);
	return date;
}


std::optional<Task> findTaskPrompt(const TaskManager& taskmanager) {
	std::string title;
	std::optional<Task> foundTask = std::nullopt;
	std::cout << "Enter Task Title:\n[Enter 0 to exit.]" << std::endl;
	while (foundTask == std::nullopt && title != EXIT_STR) {
		std::cout << "-> ";
		std::getline(std::cin, title);

		if (title == EXIT_STR) {
			break;
		}
		std::transform(title.begin(), title.end(), title.begin(), ::tolower);
		foundTask = taskmanager.findTask(title);
		if (foundTask == std::nullopt) {
			std::cout << "\n\033[31mNo Task with name '\033[0m" << title << "\033[31m' found.\033[0m" << std::endl;
		}
	}
	return foundTask;
}


std::string checkAttributePrompt(const TaskManager& taskmanager, const int& attribute, std::string& value) {

	std::vector<std::any> allowedValues;
	switch (attribute) {
		case 0: {
			std::vector<std::string> allowedValues = taskmanager.getAvailableCategories();
			break;
		}
		case 1: {
			std::vector<Priority> allowedValues = taskmanager.getAvailablePriorities();
			break;
		}
		case 2: {
			std::vector<Status> allowedValues = taskmanager.getAvailableStatuses();
			break;
		}
	}

	std::cout << "[Enter 0 to exit.]" << std::endl;
	do {
		std::cout << "-> ";
		std::getline(std::cin, value);
		if (value == EXIT_STR) {
			break;
		}
		std::transform(value.begin(), value.end(), value.begin(), ::tolower);
		if (std::find(allowedValues.begin(), allowedValues.end(), value) == allowedValues.end()) {
			std::cout << "\n\033[31mNot a valid Input.\033[0m\nAvailable Inputs: ";
			for (auto elem : allowedValues) {
				std::cout << elem << " ";
			}
			std::cout << std::endl;
		}
	} while (std::find(allowedValues.begin(), allowedValues.end(), value) != allowedValues.end() && value != EXIT_STR);
	return value;
}


void printMany(const std::vector<Task>& tasks, bool filterBool, const std::string& CatPrioStat) {
	std::cout << "\n--------------------------------------------------------------------------" << std::endl;
	if (tasks.size() == 0 && filterBool) {
		std::cout << "\n\033[31mNo Tasks with '\033[0m" << CatPrioStat << "\033[31m' found.\033[0m" << std::endl;
	}
	else {
		if (filterBool) {
			std::cout << "Tasks filtered by - " << CatPrioStat << ":\n" << std::endl;
		}
		else {
			std::cout << "Tasks:\n" << std::endl;
		}	
		for (const Task& task : tasks) {
			task.print();
		}
		std::cout << "--------------------------------------------------------------------------\n" << std::endl;
	}
}


void createJSON(TaskManager& taskmanager) {
	std::ofstream file("./data/tasks.json");
	if (!file.is_open()) {
        std::cerr << "\033[31mFailed to open tasks.json: \033[0m" << std::endl;
        return;
    }

	std::vector<Task> tasks = taskmanager.getAllTasks();

	file << "{\"tasks\": [\n";
	for (size_t i = 0; i < tasks.size(); i++) {
		file << "{\"title\": \"" << tasks[i].getTitle() << "\", ";
		file << "\"category\": \"" << tasks[i].getCategory() << "\", ";
		file << "\"dueDate\": \"" << tasks[i].getDueDate() << "\", ";
		file << "\"priority\": \"" << PrioToStr(tasks[i].getPriority()) << "\", ";
		file << "\"status\": \"" << StatToStr(tasks[i].getStatus()) << "\"}";
		if (i != tasks.size() - 1) {
			file << ",\n";
		}
	}
	file << "\n	]\n}";
	file.close();
}



int main() {

	try {
		TaskManager taskmanager;

		// /*
		// Test examples
		Task addfct("job interview", "work", "25-09-2026", Priority::High, Status::Open);
		Task custcall("haircut", "private", "17-10-2026", Priority::Medium, Status::Open);
		Task cleaning("christmas presents", "private", "23-12-2026", Priority::High, Status::InProgress);
		Task files("business meeting", "work", "07-05-2026", Priority::Low, Status::Done);
		taskmanager.addTask(addfct);
		taskmanager.addTask(custcall);
		taskmanager.addTask(cleaning);
		taskmanager.addTask(files);
		// */


		int inpChoice;
		std::string inpMenu, title, category, dueDate, priority, status, inpChange, inpSort;
		std::string emptyStr = "";
		std::vector<std::string> PrioStrVec = {"Low", "Medium", "High"};
		std::vector<std::string> StatStrVec = {"Open", "InProgress", "In Progress", "Done"};
		std::vector<std::string> ChangeStrVec = {"1", "2"};
		
		do {
			createJSON(taskmanager);

			std::cout << "\n**************************************************************************" << std::endl;
			std::cout << "Task Manager:\n1: Add Task\n2: Remove Task\n3: Find Task\n4: Change Status/Priority" <<
						"\n5: List available Tasks\n6: Filter by Category\n7: Filter by Priority" <<
						"\n8: Filter by Status\n9: Sort Tasks\n0: End\n-> ";

			std::getline(std::cin, inpMenu);
			try {
				inpChoice = stoi(inpMenu);
			}
			catch (std::exception&) {
				std::cout << "\n\033[31mInvalid Input.\033[0m" << std::endl;
				continue;
			}


			switch (inpChoice) {
				case 0: // Stop Loop
					std::cout << "\n\033[32mBye, bye!\033[0m :)\n" << std::endl;
					break;
				case 1: { // Add Task 
					std::cout << "\nAdd Task.\nEnter Task Title:\n[Enter 0 to exit.]\n-> ";
					std::getline(std::cin, title);
					if (title == EXIT_STR) { break; }
					std::transform(title.begin(), title.end(), title.begin(), ::tolower);

					std::cout << "\nEnter Task Category:\n[Enter 0 to exit.]\n-> ";
					std::getline(std::cin, category);
					if (category == EXIT_STR) { break; }
					std::transform(category.begin(), category.end(), category.begin(), ::tolower);

					std::cout << "\nEnter Task Due Date (DD-MM-YYYY):\n[Enter 0 to exit.]" << std::endl;
					dueDate = valiDATE();
					if (dueDate == EXIT_STR) { break; }

					std::cout << "\nEnter Task Priority (Low/Medium/High):" << std::endl;
					priority = checkInp(priority, PrioStrVec);
					if (priority == EXIT_STR) { break; }

					std::cout << "\nEnter Task Status (Open/InProgress/Done):" << std::endl;
					status = checkInp(status, StatStrVec);
					if (status == EXIT_STR) { break; }

					Task task(title, category, dueDate, strToPrio(priority), strToStat(status));
					if (taskmanager.addTask(task)) {
						std::cout << "\n\033[32mAdded\033[0m '" << title << "\033[32m'.\033[0m" << std::endl;
					}
					else {
						std::cout << "\n\033[31mCould not add\033[0m '" << title << "\033[31m'.\033[0m" << std::endl;
					}
					break;
				}
				case 2: { // Remove Task
					std::cout << "\nRemove Task." << std::endl;
					std::optional<Task> foundTask = findTaskPrompt(taskmanager);
					if (foundTask == std::nullopt) { break; }
					
					if (taskmanager.removeTask(foundTask->getTitle())) {
						std::cout << "\n\033[32mRemoved '\033[0m" << foundTask->getTitle() << "\033[32m'.\033[0m" << std::endl;
					}
					else {
						std::cout << "\n\033[31mCould not remove\033[0m '" << title << "\033[31m'.\033[0m" << std::endl;
					}
					break;
				}
				case 3: { // Find Task
					std::cout << "\nFind Task." << std::endl;
					std::optional<Task> foundTask = findTaskPrompt(taskmanager);
					if (foundTask == std::nullopt) { break; }

					std::cout << "\n\033[32mFound Task:\033[0m" << std::endl;
					foundTask->print();
					break;
				}
				case 4: { // Change Status/Priority
					std::cout << "\nChange Status/Priority." << std::endl;
					std::optional<Task> foundTask = findTaskPrompt(taskmanager);
					if (foundTask == std::nullopt) { break; }

					std::cout << std::endl;
					foundTask->print();
					
					std::cout << "\nChange Status (1) / Priority (2):\n";
					inpChange = checkInp(inpChange, ChangeStrVec);
					
					if (inpChange == "1") {
						std::cout << "\nEnter new Task Status (Open/InProgress/Done):\n";
						status = checkInp(status, StatStrVec);
						if (status == EXIT_STR) { break; }
						std::cout << "\n\033[32mChanged Status of '\033[0m" << foundTask.getTitle() << "\033[32m' from '\033[0m" << StatToStr(foundTask.getStatus());
						foundTask.setStatus(strToStat(status));
						std::cout << "\033[32m' to '\033[0m" << StatToStr(foundTask.getStatus()) << "\033[32m'.\033[0m" << std::endl; 
					}
					if (inpChange == "2") {
						std::cout << "\nEnter new Task Priority (Low/Medium/High):\n";
						priority = checkInp(priority, PrioStrVec);
						if (priority == EXIT_STR) { break; }
						std::cout << "\n\033[32mChanged Priority of '\033[0m" << foundTask.getTitle() << "\033[32m' from '\033[0m" << PrioToStr(foundTask.getPriority());
						foundTask.setPriority(strToPrio(priority)) ;
						std::cout << "\033[32m' to '\033[0m" << PrioToStr(foundTask.getPriority()) << "\033[32m'.\033[0m" << std::endl;
					}
					break;
				}
				case 5: { // List All Tasks
					printMany(taskmanager.getAllTasks(), false, emptyStr);
					break;
				}
				case 6: { // Filter by Category
					std::optional<std::vector<Task>> filteredTasks = std::nullopt;

					std::cout << "\nFilter Category.\nEnter Category name:" << std::endl;
					category = checkAttributePrompt(taskmanager, 0, category);
					if (category == EXIT_STR) { break; }

					filteredTasks = taskmanager.filterByCategory(category);
					printMany(filteredTasks.value(), true, category);
					break;
				}
				case 7: { // Filter by Priority
					std::cout << "\nFilter Priority.\nEnter Priority level (Low/Medium/High):" << std::endl;
					priority = checkInp(priority, PrioStrVec);
					if (priority == EXIT_STR) { break; }
					
					std::vector<Task> filteredTasks = taskmanager.filterByPriority(strToPrio(priority));
					printMany(filteredTasks, true, priority);
					break;
				}
				case 8: { // Filter by Status
					std::cout << "\nFilter Status.\nEnter Status level (Open/InProgress/Done):" << std::endl;
					status = checkInp(status, StatStrVec);
					if (status == EXIT_STR) { break; }

					std::vector<Task> filteredTasks = taskmanager.filterByStatus(strToStat(status));
					printMany(filteredTasks, true, status);
					break;
				}
				case 9: // Sort alphabetically / by Priority
					std::cout << "\nSort Alphabetically (1) / By Priority (2):\n";
					inpSort = checkInp(inpSort, ChangeStrVec);

					if (inpSort == "1") {
						taskmanager.sortByTitle();
						printMany(taskmanager.getTasks(), false, emptyStr);
					}
					else if (inpSort == "2") {
						taskmanager.sortByPriority();
						printMany(taskmanager.getTasks(), false, emptyStr);
					}
					break;
				default:
					std::cout << "\n\033[31mInvalid Input.\033[0m" << std::endl;
			}
		} while (inpChoice != 0);

		return 0;
	}

	catch (const std::runtime_error& e) {
		std::cerr << e.what() << std::endl;
		return 1;
	}
}


// TODO: tasks_sql_example.db
// TODO: Sorting also by category & status