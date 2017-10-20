#pragma once

#include "IExchange.hpp"
#include <map>
#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <limits>


using SymbolIndex = unsigned int;

// the generic order book is an association of prices versus cumulative volume
// since there's no matching of orders, there's no need to store them directly, this should increase performance and reduce memory usage
// however, if a matching engine was in place, the sequence of order insertion would matter (for the same price, first inserted first executed)
template<typename Compare>
using OrderBook = std::map<Price, Volume, Compare>;

// type to store all symbols orderbooks
template<typename Compare>
using OrderBooks = std::map<SymbolIndex, OrderBook<Compare>>;

struct Order {
	SymbolIndex symbolIdx;
	Side mSide;
	Price mPrice;
	Volume mVolume;
};

using Orders = std::unordered_map<OrderId, Order>;
using SymbolIndexMap = std::unordered_map<std::string, SymbolIndex>;

// Symbols statically defined 
const char* symbols[]{
	"AAPL",
	"GOOG",
	"MSFT"
};


class Exchange : public IExchange {
	OrderId mNextOrderId{ 0 };
	OrderBooks<std::less<Price> > mOrderBooksAsk;
	OrderBooks<std::greater<Price> > mOrderBooksBid;
	Orders mOrders;
	SymbolIndexMap mSymbolToIndexMap;

	OrderId NewOrderId() {
		return mNextOrderId++;
	}

	void InitSymbols() {
		SymbolIndex idx{ 0 };
		std::for_each(std::begin(symbols), std::end(symbols), [&](const char* v) {mSymbolToIndexMap[v] = idx++; });
	}

	// return -1 if symbol not found
	SymbolIndex Exchange::ToSymbolIndex(const std::string& symbol) {
		SymbolIndex ret = -1;
		auto it = mSymbolToIndexMap.find(symbol);
		if (it != mSymbolToIndexMap.end())
			ret = it->second;
		return ret;
	}

	template<typename T>
	InsertError ProcessOrder(T& orderBooks, Side side, const std::string& symbol, Price price, Volume volume, UserReference userReference) {
		auto ret = InsertError::OK;
		auto symbolIdx = ToSymbolIndex(symbol);
		if (symbolIdx >= 0) {
			// register the order
			OrderId orderId{ NewOrderId() };

			auto res = mOrders.emplace(orderId, Order{ symbolIdx, side, price, volume });

			// verify if the value was actually inserted... if so, register it in the order book
			if (res.second) {
				if ((std::numeric_limits<Volume>::max() - orderBooks[symbolIdx][price]) > volume) {
					orderBooks[symbolIdx][price] += volume;
				} else {
					ret = InsertError::InvalidVolume;
				}
			} else
				InsertError::SystemError;
		} else {
			ret = InsertError::SymbolNotFound;
		}

		return ret;
	}

	template<typename T>
	void InitOrderBooks(T& orderBooks) {
		for_each(begin(mSymbolToIndexMap), end(mSymbolToIndexMap), [&](const auto& v) {orderBooks[v.second] = OrderBook<T::mapped_type::key_compare>(); });
	}

public:
	
	Exchange() {
		InitSymbols();
		InitOrderBooks(mOrderBooksAsk);
		InitOrderBooks(mOrderBooksBid);
	}

	virtual void InsertOrder(
		const std::string& symbol,
		Side side,
		Price price,
		Volume volume,
		UserReference userReference
	) override {
		auto err = InsertError::OK;

		try {
			switch (side) {
				case Side::Buy:	err = ProcessOrder(mOrderBooksBid, side, symbol, price, volume, userReference);
					break;
				case Side::Sell: err = ProcessOrder(mOrderBooksAsk, side, symbol, price, volume, userReference);
					break;
				default: err = InsertError::SystemError;
			}
		
		} catch (...) {// catch everything, as it's a requirement for the application to handle / report any errors.
			err = InsertError::SystemError;
		}
		if (OnOrderInserted)
			OnOrderInserted(userReference, err, 12345);
	}

	virtual void DeleteOrder(
		OrderId orderId
	) override {

	}

private:
	
};

