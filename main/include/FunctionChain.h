
#include <any>
#include <functional>
#include <vector>
#include <typeindex>
#include <memory> // For std::unique_ptr

#include <type_traits>

// Primary template (causes errors if not specialized)
template <typename T>
struct function_traits;

// Specialization for function pointers
template <typename Out, typename In>
struct function_traits<Out (*)(In)>
{
	using ArgType = In;
	using ReturnType = Out;
};

// Specialization for member function pointers
template <typename Out, typename ClassType, typename In>
struct function_traits<Out (ClassType::*)(In)>
{
	using ArgType = In;
	using ReturnType = Out;
};

// Specialization for member function pointers
template <typename Out, typename ClassType, typename In>
struct function_traits<Out (ClassType::*)(In) const>
{
	using ArgType = In;
	using ReturnType = Out;
};

// Specialization for functors & lambdas
template <typename Functor>
struct function_traits : function_traits<decltype(&Functor::operator())>
{
};

class FunctionManager
{
private:
	// Base class hidden inside FunctionManager
	class BaseFunction
	{
	public:
		virtual ~BaseFunction() = default;
		virtual std::any invoke(const std::any &input) = 0;
		virtual std::type_index inputType() const = 0;
		virtual std::type_index outputType() const = 0;
	};

	// Function wrapper inside FunctionManager
	template <typename In, typename Out>
	class FunctionWrapper : public BaseFunction
	{
		std::function<Out(In)> func;

	public:
		FunctionWrapper(std::function<Out(In)> f) : func(std::move(f)) {}

		std::any invoke(const std::any &input) override
		{
			if (input.type() != typeid(In))
			{
				throw std::runtime_error("Invalid input type");
			}
			return func(std::any_cast<In>(input));
		}

		std::type_index inputType() const override { return typeid(In); }
		std::type_index outputType() const override { return typeid(Out); }
	};

	std::vector<std::unique_ptr<BaseFunction>> functions; // Safe storage

public:
	template <typename In, typename Out>
	void addFunction(std::function<Out(In)> func)
	{
		functions.push_back(std::make_unique<FunctionWrapper<In, Out>>(std::move(func)));
	}

	// 1️⃣ Function pointer overload
	template <typename In, typename Out>
	void addFunction(Out (*func)(In))
	{
		addFunction(std::function<Out(In)>(func));
	}

	// 2️⃣ Functor/Lambda overload (extracts input/output types)
	template <typename Functor>
	void addFunction(Functor func)
	{
		using In = typename function_traits<Functor>::ArgType;
		using Out = typename function_traits<Functor>::ReturnType;
		addFunction(std::function<Out(In)>(std::move(func)));
	}

	// 3️⃣ std::reference_wrapper<T> overload (fix for stateful objects!)
	template <typename Functor>
	void addFunction(std::reference_wrapper<Functor> func)
	{
		addFunction(func.get()); // Unwrap reference and process normally
	}

	std::any executeChain(const std::any &initialValue)
	{
		std::any value = initialValue;
		std::type_index currentType = value.type();

		for (auto &func : functions)
		{
			if (func->inputType() == currentType)
			{
				value = func->invoke(value);
				currentType = func->outputType();
			}
			else
			{
				throw std::runtime_error("Skipping function due to type mismatch");
			}
		}
		return value;
	}
};

// Example Functions
bool floatToBool(float x) { return x > 0.5f; }
float boolToFloat(bool b) { return b ? 1.0f : 0.0f; }

// Low-pass filter class
class LowPassFilter
{
	float alpha;
	float state;

public:
	LowPassFilter(float alpha, float initialState = 0.0f)
		: alpha(alpha), state(initialState)
	{
	}

	float operator()(float input)
	{
		state = alpha * input + (1 - alpha) * state;
		return state;
	}
};