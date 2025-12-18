#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <optional>

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
				case Priority::Low: std::cout << ", Priority: Low"; break;
				case Priority::Medium: std::cout << ", Priority: Medium"; break;
				case Priority::High: std::cout << ", Priority: High"; break;
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
		std::vector<Task> tasks;

	public:
		std::vector<Task>& getTasks() { return tasks; }

		void addTask(const Task& task) {
			tasks.push_back(task);
		}

		void removeTask(const std::string& title) {
			tasks.erase(
			std::remove_if(tasks.begin(), tasks.end(), [&](const Task& task) -> bool {return task.getTitle() == title;}),
			tasks.end());

		}

		std::optional<size_t> findTaskIndex(const std::string& title) const {
			for (size_t i = 0; i < tasks.size(); i++) {
				if (tasks[i].getTitle() == title) {
					return i;
				}
			}
			return std::nullopt;
		}

		std::vector<Task> filterByCategory(const std::string& cat) const {
			std::vector<Task> filterCategoryTasks;
			for (const Task& task : tasks) {
				if (task.getCategory() == cat) { 
					filterCategoryTasks.push_back(task);
				}
			}
			return filterCategoryTasks;
		}

		std::vector<Task> filterByPriority(Priority prio) const {
			std::vector<Task> filterPriorityTasks;
			for (const Task& task : tasks) {
				if (task.getPriority() == prio) {
					filterPriorityTasks.push_back(task);
				}
			}
			return filterPriorityTasks;
		}

		std::vector<Task> filterByStatus(Status stat) const {
			std::vector<Task> filterStatusTasks;
			for (const Task& task : tasks) {
				if (task.getStatus() == stat) {
					filterStatusTasks.push_back(task);
				}
			}
			return filterStatusTasks;
		}

		void sortByTitle() {
			std::sort(tasks.begin(), tasks.end(),
			[](const Task &taskone, const Task &tasktwo) -> bool {return (taskone.getTitle() < tasktwo.getTitle());});
		}

		void sortByPriority() {
			std::sort(tasks.begin(), tasks.end(),
			[](const Task& taskone, const Task& tasktwo) -> bool {return static_cast<int>(taskone.getPriority()) < static_cast<int>(tasktwo.getPriority());});
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
	throw std::invalid_argument("Invalid Priority: " + inp);
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
	throw std::invalid_argument("Invalid Status: " + inp);
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
	throw std::invalid_argument("Invalid Priority.");
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
	throw std::invalid_argument("Invalid Status.");
}


constexpr const char* EXIT_STR = "0";

std::optional<size_t> checkTaskInp(TaskManager& taskmanager) {
	std::string title;
	std::optional<size_t> foundTaskIndex = std::nullopt;
	std::cout << "Enter Task Title:\n[Enter 0 to exit.]" << std::endl;
	while (foundTaskIndex == std::nullopt && title != EXIT_STR) {
		std::cout << "-> ";
		std::getline(std::cin, title);

		if (title == EXIT_STR) {
			break;
		}
		std::transform(title.begin(), title.end(), title.begin(), ::tolower);
		foundTaskIndex = taskmanager.findTaskIndex(title);
		if (foundTaskIndex == std::nullopt) {
			std::cout << "\nNo Task with name '" << title << "' found." << std::endl;
		}
	}
	return foundTaskIndex;
}

std::string checkInp(std::string& inpStr, const std::vector<std::string>& allowVec) {
	std::vector<std::string> allowVecLow;
	for (std::string elem : allowVec) {
		std::transform(elem.begin(), elem.end(), elem.begin(), ::tolower);
		allowVecLow.push_back(elem);
	}
	std::cout << "[Enter 0 to exit.]" << std::endl;
	do {
		std::cout << "-> ";
		std::getline(std::cin, inpStr);
		if (inpStr == EXIT_STR) {
			break;
		}
		std::transform(inpStr.begin(), inpStr.end(), inpStr.begin(), ::tolower);
		if (std::find(allowVecLow.begin(), allowVecLow.end(), inpStr) == allowVecLow.end()) {
			std::cout << "\nNot a valid Input.\nAvailable Inputs: ";
			for (std::string elem : allowVec) {
				std::cout << elem << " ";
			}
			std::cout << std::endl;
		}
	} while (std::find(allowVecLow.begin(), allowVecLow.end(), inpStr) == allowVecLow.end() && inpStr != EXIT_STR);
	return inpStr;
}

void printMany(const std::vector<Task>& tasks, bool filterBool, const std::string& CatPrioStat) {
	std::cout << "\n--------------------------------------------------------------------------" << std::endl;
	if (tasks.size() == 0 && filterBool) {
		std::cout << "\nNo Tasks with '" << CatPrioStat << "' found." << std::endl;
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


int main() {

	TaskManager taskmanager;


	// Test examples
	Task dennis("dennis files", "it", "28-02-2028", Priority::High, Status::Open);
	Task finja("finja", "service", "25-12-2025", Priority::High, Status::Open);
	Task finja2("finja2", "service", "25-12-2025", Priority::Medium, Status::InProgress);
	Task xaver("xavi", "humor", "23-06-1998", Priority::Low, Status::Done);
	taskmanager.addTask(finja2);
	taskmanager.addTask(xaver);
	taskmanager.addTask(finja);
	taskmanager.addTask(dennis);

	int inpChoice;
	std::string title, category, dueDate, priority, status, inpChange, inpSort;
	std::string emptyStr = "";
	std::vector<std::string> PrioStrVec = {"Low", "Medium", "High"};
	std::vector<std::string> StatStrVec = {"Open", "InProgress", "In Progress", "Done"};
	std::vector<std::string> ChangeStrVec = {"1", "2"};
	
	do {
		std::cout << "\n**************************************************************************" << std::endl;
		std::cout << "Task Manager:\n1: Add Task\n2: Remove Task\n3: Find Task\n4: Change Status/Priority" <<
					 "\n5: List available Tasks\n6: Filter by Category\n7: Filter by Priority" <<
					 "\n8: Filter by Status\n9: Sort Tasks\n0: End\n-> ";

		std::cin >> inpChoice;
		std::cin.ignore();

		switch (inpChoice) {
			case 0: // Stop Loop
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

				std::cout << "\nEnter Task Due Date (DD-MM-YYYY):\n[Enter 0 to exit.]\n-> ";
				std::getline(std::cin, dueDate);
				if (dueDate == EXIT_STR) { break; }

				std::cout << "\nEnter Task Priority (Low/Medium/High):" << std::endl;
				priority = checkInp(priority, PrioStrVec);
				if (priority == EXIT_STR) { break; }

				std::cout << "\nEnter Task Status (Open/InProgress/Done):" << std::endl;
				status = checkInp(status, StatStrVec);
				if (status == EXIT_STR) { break; }

				Task task(title, category, dueDate, strToPrio(priority), strToStat(status));
				taskmanager.addTask(task);
				std::cout << "\nAdded " << title << "." << std::endl;
				break;
			}
			case 2: { // Remove Task
				std::cout << "\nRemove Task." << std::endl;
				std::optional<size_t> foundTaskIndex = checkTaskInp(taskmanager);
				if (foundTaskIndex == std::nullopt) { break; }
				Task& foundTask = taskmanager.getTasks()[foundTaskIndex.value()];

				std::cout << "\nRemoved " << foundTask.getTitle() << "." << std::endl;
				taskmanager.removeTask(foundTask.getTitle());
				break;
			}
			case 3: { // Find Task
				std::cout << "\nFind Task." << std::endl;
				std::optional<size_t> foundTaskIndex = checkTaskInp(taskmanager);
				if (foundTaskIndex == std::nullopt) { break; }
				Task& foundTask = taskmanager.getTasks()[foundTaskIndex.value()];

				std::cout << "\nFound Task:" << std::endl;
				foundTask.print();
				break;
			}
			case 4: { // Change Status/Priority
				std::cout << "\nChange Status/Priority." << std::endl;
				std::optional<size_t> foundTaskIndex = checkTaskInp(taskmanager);
				if (foundTaskIndex == std::nullopt) { break; }
				Task& foundTask = taskmanager.getTasks()[foundTaskIndex.value()];

				std::cout << std::endl;
				foundTask.print();
				
				std::cout << "\nChange Status (1) / Priority (2):\n";
				inpChange = checkInp(inpChange, ChangeStrVec);
				
				if (inpChange == "1") {
					std::cout << "\nEnter new Task Status (Open/InProgress/Done):\n";
					status = checkInp(status, StatStrVec);
					if (status == EXIT_STR) { break; }
					std::cout << "\nChanged Status of '" << foundTask.getTitle() << "' from '" << StatToStr(foundTask.getStatus());
					foundTask.setStatus(strToStat(status));
					std::cout << "' to '" << StatToStr(foundTask.getStatus()) << "'." << std::endl; 
				}
				if (inpChange == "2") {
					std::cout << "\nEnter new Task Priority (Low/Medium/High):\n";
					priority = checkInp(priority, PrioStrVec);
					if (priority == EXIT_STR) { break; }
					std::cout << "\nChanged Priority of '" << foundTask.getTitle() << "' from '" << PrioToStr(foundTask.getPriority());
					foundTask.setPriority(strToPrio(priority)) ;
					std::cout << "' to '" << PrioToStr(foundTask.getPriority()) << "'." << std::endl;
				}
				break;
			}
			case 5: { // List All Tasks
				printMany(taskmanager.getTasks(), false, emptyStr);
				break;
			}
			case 6: { // Filter by Category		
				std::vector<std::string> CatStrVec;
				for (const Task& task : taskmanager.getTasks()) {
					if (std::find(CatStrVec.begin(), CatStrVec.end(), task.getCategory()) == CatStrVec.end()) {
						CatStrVec.push_back(task.getCategory());
					}
				}
				std::cout << "\nFilter Category.\nEnter Category name:" << std::endl;
				category = checkInp(category, CatStrVec);
				if (category == EXIT_STR) { break; }

				std::vector<Task> filteredTasks = taskmanager.filterByCategory(category);
				printMany(filteredTasks, true, category);
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
				std::cout << "\nNo valid Input." << std::endl;
		}
	} while (inpChoice != 0);

return 0;
}

// Can enter chars/strings in main menu -> Will immidiately close program (only integers fine -> stop or exec)
