#include "stdafx.h"
#include "CppUnitTest.h"
#include "Exchange.hpp"
#include "IExchange.hpp"
#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

using namespace std;
using namespace std::placeholders;

namespace unit_tests
{		
	void CallBackOrderInserted(UserReference userReference,	InsertError insertError, OrderId orderId) {

	}

	class ExchangeActivity {
	public:
		void CallbackOrderInserted(UserReference userReference, InsertError insertError, OrderId orderId) {
			mLastUserReference = userReference;
			mInsertError = insertError;
			mOrderId = orderId;
		}
	public:
		UserReference mLastUserReference{ -1 };
		InsertError mInsertError{ InsertError::InvalidPrice };// InvalidPrice because it's never used
		OrderId mOrderId{ -1 };

	};

	TEST_CLASS(UnitTests)
	{
	public:
		
		TEST_METHOD(CreateDestroyExchangeClass)
		{
			Exchange exchange;
		}

		TEST_METHOD(CreateDestroyExchangeInterface) {
			
			unique_ptr<IExchange> pEx = make_unique<Exchange>();
		}

		TEST_METHOD(BasicUsage) {
			unique_ptr<IExchange> pEx = make_unique<Exchange>();
			pEx->InsertOrder("AAPL", Side::Buy, 15, 1000, 123);
		}

		TEST_METHOD(PassVolumeLimit) {
			unique_ptr<IExchange> pEx = make_unique<Exchange>();
			
			ExchangeActivity activity;

			pEx->OnOrderInserted = std::bind(&ExchangeActivity::CallbackOrderInserted, &activity, _1, _2, _3);
			pEx->InsertOrder("AAPL", Side::Buy, 15, 1000, 123);
			
		}

	};
}