#pragma once

namespace Expanse::Game
{
	struct World;

	class ISystem
	{
	public:
		explicit ISystem(World& w) : world(w) {}
		virtual ~ISystem() = default;

		virtual void Update() = 0;
	protected:
		World& world;
	};


	class FunctionSystem : public ISystem
	{
	public:
		using Func = void (*) (World&);

		FunctionSystem(World& w, Func fun)
			: ISystem(w)
			, func(fun)
		{}

		void Update() override
		{
			func(world);
		}
	private:
		Func func;
	};


	class SystemCollection : public ISystem
	{
		using SystemPtr = std::unique_ptr<ISystem>;
	public:
		explicit SystemCollection(World& w) : ISystem(w) {}

		template<class S = FunctionSystem, typename... Args> requires std::is_base_of_v<ISystem, S>
		S* AddSystem(Args&&... args)
		{
			systems.push_back(std::make_unique<S>(world, std::forward<Args>(args)...));
			return static_cast<S*>(systems.back().get());
		}

		void Update() override
		{
			for (auto& sys : systems) {
				sys->Update();
			}
		}
	private:
		std::vector<SystemPtr> systems;
	};
}