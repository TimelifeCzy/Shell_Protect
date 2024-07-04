#include <mutex>
namespace ustdex
{
	template <typename T>
	class Singleton //: private T
	{
	private:
		Singleton();
		~Singleton();

	public:
		static T* instance();
		static void release();

	private:
		static std::mutex lock_;
		static T* instance_;
	};

	template <typename T>
	Singleton<T>::~Singleton()
	{
		release();
	}

	template <class T>
	std::mutex Singleton<T>::lock_;

	template <class T>
	T* Singleton<T>::instance_ = nullptr;

	template <class T>
	T* Singleton<T>::instance()
	{
		if (instance_ == nullptr)
		{
			lock_.lock();
			if (instance_ == nullptr)
				instance_ = new T;
			lock_.unlock();
		}
		return instance_;
	}

	template <class T>
	void Singleton<T>::release()
	{
		if (instance_ != nullptr)
		{
			lock_.lock();
			if (instance_ != nullptr)
			{
				delete instance_;
				instance_ = nullptr;
			}
			lock_.unlock();
		}
	}

}  // namespace ustdex