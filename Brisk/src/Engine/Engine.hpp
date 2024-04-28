namespace Brisk 
{
	static class Engine 
	{
	public:
		static void Init();
	private:
		Engine();

	public:
		static Engine* Instance;
	private:
	};
}