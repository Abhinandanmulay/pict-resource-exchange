// PICT Resource Exchange - Frontend Application Logic
// Determine API base: If served directly by the native C++ engine (port 8080), use origin.
// If served via VS Code Live Server (port 5500, etc.) or file://, connect to C++ backend at http://localhost:8080
const BACKEND_PORT = '8080';
const BACKEND_HOST = (window.location.hostname && window.location.hostname !== '') ? window.location.hostname : 'localhost';
const API_BASE = (window.location.port === BACKEND_PORT)
  ? window.location.origin
  : `http://${BACKEND_HOST}:${BACKEND_PORT}`;

// State management
let allItems = [];
let allStudents = [];
let activeStudent = null;
let currentCategory = 'All';
let currentSearch = '';
let currentAvailability = '';
let currentSort = 'default';
let currentListingTypeFilter = 'all'; // 'all', 'rent', 'sell'
document.documentElement.setAttribute('data-theme', 'dark');
localStorage.removeItem('pict_theme');

// DOM Elements
const itemsGrid = document.getElementById('items-grid');
const requestsGrid = document.getElementById('requests-grid');
const recentGrid = document.getElementById('recent-grid');
const searchInput = document.getElementById('search-input');
const clearSearchBtn = document.getElementById('clear-search-btn');
const activeStudentSelect = document.getElementById('active-student-select');
const categoryPills = document.getElementById('category-pills');
const availabilityFilter = document.getElementById('availability-filter');
const sortSelect = document.getElementById('sort-select');
const resultsCountText = document.getElementById('results-count-text');

// Hero Stats
const statTotal = document.getElementById('stat-total-items');
const statAvailable = document.getElementById('stat-available-items');
const statWaitlist = document.getElementById('stat-waitlist-count');
const statRequests = document.getElementById('stat-requests-count');
const navBorrowedBadge = document.getElementById('nav-borrowed-badge');

// View Tabs
const tabCatalog = document.getElementById('tab-catalog');
const tabRequests = document.getElementById('tab-requests');
const tabRecent = document.getElementById('tab-recent');
const catalogView = document.getElementById('catalog-view');
const requestsView = document.getElementById('requests-view');
const recentView = document.getElementById('recent-view');

// Modals & Drawers
const lendModal = document.getElementById('lend-modal-overlay');
const requestModal = document.getElementById('request-modal-overlay');
const itemModal = document.getElementById('item-modal-overlay');
const statsModal = document.getElementById('stats-modal-overlay');
const authModal = document.getElementById('auth-modal-overlay');
const confirmModal = document.getElementById('confirm-modal-overlay');
const feedbackModal = document.getElementById('feedback-modal-overlay');
const btnCloseFeedbackModal = document.getElementById('btn-close-feedback-modal');
const fulfillModal = document.getElementById('fulfill-modal-overlay');
const btnCloseFulfillModal = document.getElementById('btn-close-fulfill-modal');
const buyModal = document.getElementById('buy-modal-overlay');
const btnCloseBuyModal = document.getElementById('btn-close-buy-modal');
const dashboardDrawer = document.getElementById('dashboard-drawer');
const drawerOverlay = document.getElementById('drawer-overlay');

// Feedback & Rating state
let currentReturningItemId = null;
let currentFeedbackRating = 5;

// Wanted Board state
let allRequests = [];
let currentRequestFilter = 'all'; // 'all', 'open', 'fulfilled'

const RATING_DESCRIPTIONS = {
  1: "★☆☆☆☆ Poor condition / issues encountered",
  2: "★★☆☆☆ Below expectations",
  3: "★★★☆☆ Average / acceptable condition",
  4: "★★★★☆ Good condition & smooth handover",
  5: "★★★★★ Excellent condition & helpful lender"
};

// Auth elements
const btnOpenLoginModal = document.getElementById('btn-open-login-modal');
const btnCloseAuthModal = document.getElementById('btn-close-auth-modal');
const btnSwitchAccount = document.getElementById('btn-switch-account');
const navUserProfile = document.getElementById('nav-user-profile');
const navUserName = document.getElementById('nav-user-name');
const navUserRoll = document.getElementById('nav-user-roll');
const navAvatar = document.getElementById('nav-avatar');
const tabAuthLogin = document.getElementById('tab-auth-login');
const tabAuthRegister = document.getElementById('tab-auth-register');
const authContentLogin = document.getElementById('auth-content-login');
const authContentRegister = document.getElementById('auth-content-register');
const loginForm = document.getElementById('login-form');
const registerForm = document.getElementById('register-form');
const quickStudentGrid = document.getElementById('quick-student-grid');
let pendingCancelItemId = null;

// Category Metadata & Clean Vector Icons (No photographic images)
const CATEGORY_META = {
  'Drawing Tools': {
    themeClass: 'cat-drawing',
    iconSvg: `<svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2L2 7l10 5 10-5-10-5zM2 17l10 5 10-5M2 12l10 5 10-5"/></svg>`,
    badgeLabel: 'Drawing Tools'
  },
  'Books': {
    themeClass: 'cat-books',
    iconSvg: `<svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M4 19.5A2.5 2.5 0 0 1 6.5 17H20"/><path d="M6.5 2H20v20H6.5A2.5 2.5 0 0 1 4 19.5v-15A2.5 2.5 0 0 1 6.5 2z"/></svg>`,
    badgeLabel: 'Textbooks'
  },
  'Electronics': {
    themeClass: 'cat-electronics',
    iconSvg: `<svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="4" y="4" width="16" height="16" rx="2"/><rect x="9" y="9" width="6" height="6"/><line x1="9" y1="1" x2="9" y2="4"/><line x1="15" y1="1" x2="15" y2="4"/><line x1="9" y1="20" x2="9" y2="23"/><line x1="15" y1="20" x2="15" y2="23"/><line x1="20" y1="9" x2="23" y2="9"/><line x1="20" y1="14" x2="23" y2="14"/><line x1="1" y1="9" x2="4" y2="9"/><line x1="1" y1="14" x2="4" y2="14"/></svg>`,
    badgeLabel: 'Electronics'
  },
  'Stationery': {
    themeClass: 'cat-stationery',
    iconSvg: `<svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="4" y="2" width="16" height="20" rx="2"/><line x1="8" y1="6" x2="16" y2="6"/><line x1="16" y1="14" x2="16" y2="18"/><path d="M16 10h.01"/><path d="M12 10h.01"/><path d="M8 10h.01"/><path d="M12 14h.01"/><path d="M8 14h.01"/><path d="M12 18h.01"/><path d="M8 18h.01"/></svg>`,
    badgeLabel: 'Stationery'
  },
  'Aprons': {
    themeClass: 'cat-aprons',
    iconSvg: `<svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 22s8-4 8-10V5l-8-3-8 3v7c0 6 8 10 8 10z"/></svg>`,
    badgeLabel: 'Workshop Apron'
  }
};

function getCategoryMeta(cat) {
  return CATEGORY_META[cat] || {
    themeClass: 'cat-default',
    iconSvg: `<svg width="22" height="22" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><rect x="2" y="7" width="20" height="14" rx="2" ry="2"/><path d="M16 21V5a2 2 0 0 0-2-2h-4a2 2 0 0 0-2 2v16"/></svg>`,
    badgeLabel: cat || 'Resource'
  };
}

// ---------------------------------------------------------------------------
// Initialization
// ---------------------------------------------------------------------------
async function init() {
  document.documentElement.setAttribute('data-theme', 'dark');
  bindEvents();
  await loadStudents();
  await loadCatalogItems();
  await loadStats();
  await loadRequests();
}

// ---------------------------------------------------------------------------
// API Calls
// ---------------------------------------------------------------------------
async function loadStudents() {
  try {
    const res = await fetch(`${API_BASE}/api/students`);
    if (!res.ok) throw new Error('Failed to load students');
    allStudents = await res.json();
    populateStudentSelect();

    // Check localStorage for persisted student session
    const savedRoll = localStorage.getItem('pict_exchange_active_roll');
    let target = null;
    if (savedRoll) {
      target = allStudents.find(s => s.rollNo === savedRoll);
    }
    if (!target && allStudents.length > 0) {
      target = allStudents[0];
    }
    if (target) {
      setLoggedInStudent(target);
    }
  } catch (err) {
    console.error('Error loading students:', err);
  }
}

async function loadCatalogItems() {
  try {
    let url = `${API_BASE}/api/items?`;
    if (currentSearch) url += `query=${encodeURIComponent(currentSearch)}&`;
    if (currentCategory && currentCategory !== 'All') url += `category=${encodeURIComponent(currentCategory)}&`;
    if (currentAvailability) url += `availability=${encodeURIComponent(currentAvailability)}&`;

    const res = await fetch(url);
    if (!res.ok) throw new Error('Failed to load catalog');
    allItems = await res.json();
    renderCatalog();
  } catch (err) {
    console.error('Error loading catalog items:', err);
    itemsGrid.innerHTML = `
      <div class="empty-state" style="padding: 40px 20px; text-align: center; grid-column: 1 / -1;">
        <p style="font-size: 1.1rem; font-weight: 600; color: #f87171; margin-bottom: 8px;">⚠️ Unable to connect to C++ Backend (${API_BASE})</p>
        <p style="color: var(--text-muted); font-size: 0.9rem;">Make sure <code>pict_exchange.exe</code> is running, or visit <a href="http://localhost:8080/" style="color: #60a5fa; text-decoration: underline;">http://localhost:8080/</a> directly.</p>
      </div>
    `;
  }
}

async function loadRequests() {
  try {
    const res = await fetch(`${API_BASE}/api/requests`);
    if (!res.ok) throw new Error('Failed to load requests');
    allRequests = await res.json();
    renderRequests();
  } catch (err) {
    console.error('Error loading requests:', err);
  }
}

async function loadRecentlyViewed() {
  if (!activeStudent) return;
  try {
    const res = await fetch(`${API_BASE}/api/student/recent?roll=${encodeURIComponent(activeStudent.rollNo)}`);
    if (!res.ok) throw new Error('Failed to load recent items');
    const recentItems = await res.json();
    renderRecentlyViewed(recentItems);
  } catch (err) {
    console.error('Error loading recent items:', err);
  }
}

async function loadStats() {
  try {
    const res = await fetch(`${API_BASE}/api/stats`);
    if (!res.ok) throw new Error('Failed to load stats');
    const stats = await res.json();

    statTotal.textContent = stats.totalItems;
    statAvailable.textContent = stats.availableItems;
    statWaitlist.textContent = stats.waitlistQueueCount;
    statRequests.textContent = stats.totalRequests;

    // DS Inspector Modal stats
    document.getElementById('ds-stat-items').textContent = stats.totalItems;
    document.getElementById('ds-stat-students').textContent = stats.totalStudents;
    document.getElementById('ds-stat-queue').textContent = stats.waitlistQueueCount;
    document.getElementById('ds-stat-undo').textContent = stats.undoStackDepth;
    document.getElementById('ds-stat-requests').textContent = stats.totalRequests;
  } catch (err) {
    console.error('Error loading stats:', err);
  }
}

// ---------------------------------------------------------------------------
// Student Profile Management
// ---------------------------------------------------------------------------
function setLoggedInStudent(student) {
  if (!student) return;
  activeStudent = student;
  localStorage.setItem('pict_exchange_active_roll', student.rollNo);

  if (activeStudentSelect) {
    activeStudentSelect.value = student.rollNo;
  }

  updateActiveStudentUI();
  renderQuickStudents();

  if (recentView && recentView.classList.contains('active')) {
    loadRecentlyViewed();
  }
}

function updateActiveStudentUI() {
  if (!activeStudent) {
    if (navUserProfile) navUserProfile.style.display = 'none';
    if (btnOpenLoginModal) btnOpenLoginModal.style.display = 'inline-flex';
    return;
  }

  if (btnOpenLoginModal) btnOpenLoginModal.style.display = 'none';
  if (navUserProfile) navUserProfile.style.display = 'flex';

  const count = activeStudent.borrowedItemIds ? activeStudent.borrowedItemIds.length : 0;
  navBorrowedBadge.textContent = count;
  navBorrowedBadge.style.display = count > 0 ? 'inline-block' : 'none';

  const initials = activeStudent.name.split(' ').map(n => n[0]).join('').substring(0, 2).toUpperCase();
  if (navAvatar) navAvatar.textContent = initials;
  if (navUserName) navUserName.textContent = activeStudent.name;
  if (navUserRoll) navUserRoll.textContent = `Roll: ${activeStudent.rollNo}`;

  // Drawer info
  const drawerAvatar = document.getElementById('drawer-avatar');
  if (drawerAvatar) drawerAvatar.textContent = initials;
  const drawerStudentName = document.getElementById('drawer-student-name');
  if (drawerStudentName) drawerStudentName.textContent = activeStudent.name;
  const drawerStudentMeta = document.getElementById('drawer-student-meta');
  if (drawerStudentMeta) drawerStudentMeta.textContent = `Roll: ${activeStudent.rollNo} • ${activeStudent.department} (Year ${activeStudent.year})`;
  const drawerRollDisplay = document.getElementById('drawer-roll-display');
  if (drawerRollDisplay) drawerRollDisplay.textContent = activeStudent.rollNo;
  const drawerEmailDisplay = document.getElementById('drawer-email-display');
  if (drawerEmailDisplay) drawerEmailDisplay.textContent = activeStudent.email || `${activeStudent.rollNo}@pict.edu`;
}

function renderQuickStudents() {
  if (!quickStudentGrid) return;
  quickStudentGrid.innerHTML = allStudents.map(s => {
    const isCurrent = activeStudent && activeStudent.rollNo === s.rollNo;
    return `
      <div class="quick-student-chip" onclick="handleQuickLogin('${escapeHtml(s.rollNo)}')" style="${isCurrent ? 'border-color:#38bdf8; background:rgba(56,189,248,0.15);' : ''}">
        <span class="quick-chip-name">${escapeHtml(s.name)} ${isCurrent ? '✓' : ''}</span>
        <span class="quick-chip-meta">Roll: ${escapeHtml(s.rollNo)} • ${escapeHtml(s.department)}</span>
      </div>
    `;
  }).join('');
}

function populateStudentSelect() {
  if (activeStudentSelect) {
    activeStudentSelect.innerHTML = '';
    allStudents.forEach((student) => {
      const opt = document.createElement('option');
      opt.value = student.rollNo;
      opt.textContent = `${student.name} (${student.rollNo})`;
      activeStudentSelect.appendChild(opt);
    });
  }
  renderQuickStudents();
}


// ---------------------------------------------------------------------------
// Listing Intent & Selling Helpers
// ---------------------------------------------------------------------------
window.handleIntentChange = function(intent) {
  const rentFields = document.getElementById('lend-rent-fields');
  const sellFields = document.getElementById('lend-sell-fields');
  const priceInput = document.getElementById('lend-price');
  const depositInput = document.getElementById('lend-deposit');
  const daysInput = document.getElementById('lend-days');
  const sellPriceInput = document.getElementById('lend-sell-price');

  if (intent === 'RENT') {
    if (rentFields) rentFields.style.display = 'flex';
    if (sellFields) sellFields.style.display = 'none';
    if (priceInput) priceInput.required = true;
    if (depositInput) depositInput.required = true;
    if (daysInput) daysInput.required = true;
    if (sellPriceInput) sellPriceInput.required = false;
  } else if (intent === 'SELL') {
    if (rentFields) rentFields.style.display = 'none';
    if (sellFields) sellFields.style.display = 'block';
    if (priceInput) priceInput.required = false;
    if (depositInput) depositInput.required = false;
    if (daysInput) daysInput.required = false;
    if (sellPriceInput) sellPriceInput.required = true;
  } else if (intent === 'BOTH') {
    if (rentFields) rentFields.style.display = 'flex';
    if (sellFields) sellFields.style.display = 'block';
    if (priceInput) priceInput.required = true;
    if (depositInput) depositInput.required = true;
    if (daysInput) daysInput.required = true;
    if (sellPriceInput) sellPriceInput.required = true;
  }
};

window.setSellPrice = function(val) {
  const input = document.getElementById('lend-sell-price');
  if (input) input.value = val;
};

// ---------------------------------------------------------------------------
// Rendering Functions
// ---------------------------------------------------------------------------
function renderCatalog() {
  let items = [...allItems];

  // Filter by Listing Type (All, Rent, Sell)
  if (currentListingTypeFilter === 'rent') {
    items = items.filter(i => (i.listingType === 'RENT' || i.listingType === 'BOTH') && !i.isSold);
  } else if (currentListingTypeFilter === 'sell') {
    items = items.filter(i => (i.listingType === 'SELL' || i.listingType === 'BOTH') && !i.isSold);
  }

  // Sorting
  if (currentSort === 'price-low') {
    items.sort((a, b) => (a.borrowPrice || 0) - (b.borrowPrice || 0));
  } else if (currentSort === 'deposit-low') {
    items.sort((a, b) => a.depositAmount - b.depositAmount);
  } else if (currentSort === 'deposit-high') {
    items.sort((a, b) => b.depositAmount - a.depositAmount);
  } else if (currentSort === 'sell-price-low') {
    items.sort((a, b) => (a.sellPrice || 0) - (b.sellPrice || 0));
  } else if (currentSort === 'sell-price-high') {
    items.sort((a, b) => (b.sellPrice || 0) - (a.sellPrice || 0));
  } else if (currentSort === 'rating-high') {
    items.sort((a, b) => (b.averageRating || 0) - (a.averageRating || 0));
  } else if (currentSort === 'days-high') {
    items.sort((a, b) => b.maxDays - a.maxDays);
  }

  resultsCountText.textContent = `Showing ${items.length} ${items.length === 1 ? 'item' : 'items'}`;

  if (items.length === 0) {
    itemsGrid.innerHTML = `
      <div style="grid-column: 1 / -1; text-align: center; padding: 48px 20px;">
        <h3 style="margin-bottom: 8px; color: #94a3b8;">No matching resources found</h3>
        <p style="color: #64748b; font-size: 0.9rem;">Try searching for another keyword or change your listing filter!</p>
      </div>
    `;
    return;
  }

  itemsGrid.innerHTML = items.map(item => {
    const isAvail = item.isAvailable;
    const isSold = item.isSold;
    const isSellOnly = item.listingType === 'SELL';
    const isBoth = item.listingType === 'BOTH';
    const sellPrice = item.sellPrice || 0;
    const waitCount = item.waitlistCount || (item.waitlist ? item.waitlist.length : 0);
    const depositText = item.depositAmount === 0 ? 'FREE' : `₹${item.depositAmount.toFixed(0)}`;
    const feeText = (!item.borrowPrice || item.borrowPrice === 0) ? 'FREE' : `₹${item.borrowPrice.toFixed(0)}`;
    const hasReviews = item.reviewCount && item.reviewCount > 0;
    const ratingDisplay = hasReviews 
      ? `★ ${item.averageRating.toFixed(1)} <span class="rating-count">(${item.reviewCount})</span>`
      : `★ New <span class="rating-count">(No reviews)</span>`;
    const catMeta = getCategoryMeta(item.category);

    return `
      <div class="item-card" data-id="${item.id}">
        <!-- Category Banner (No external photo images) -->
        <div class="card-category-banner">
          <div style="display:flex; align-items:center; gap:10px;">
            <div class="cat-icon-wrap ${catMeta.themeClass}">
              ${catMeta.iconSvg}
            </div>
            <div>
              <span class="card-badge-category">${escapeHtml(item.category)}</span>
              <span class="card-item-id">${escapeHtml(item.id)}</span>
            </div>
          </div>
          <div>
            ${isSold ? `
              <span class="card-badge-sold">✓ SOLD</span>
            ` : isSellOnly ? `
              <span class="card-badge-sale">🏷️ For Sale: ₹${sellPrice.toFixed(0)}</span>
            ` : isBoth ? `
              <span class="card-badge-both">💎 Rent / Buy</span>
            ` : `
              <span class="card-badge-status ${isAvail ? 'badge-available' : 'badge-borrowed'}">
                ${isAvail ? '● Available' : `● Borrowed (${waitCount} queue)`}
              </span>
            `}
          </div>
        </div>

        <div class="card-body">
          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:8px;">
            <span class="card-condition-pill" style="margin-bottom:0;">${escapeHtml(item.condition)}</span>
            <div class="card-rating-chip" title="${hasReviews ? `${item.reviewCount} verified review(s)` : 'New item on exchange'}">
              ${ratingDisplay}
            </div>
          </div>
          <h3 class="card-title" title="${escapeHtml(item.title)}">${escapeHtml(item.title)}</h3>
          <p class="card-desc">${escapeHtml(item.description)}</p>

          <div class="card-pricing-row">
            ${isSellOnly ? `
              <div class="price-chip fee" style="flex:1; border-color:rgba(16,185,129,0.3); background:rgba(16,185,129,0.08);">
                <span class="price-val" style="color:#10b981;">₹${sellPrice.toFixed(0)}</span>
                <span class="price-sub">Permanent Purchase Price</span>
              </div>
            ` : isBoth ? `
              <div class="price-chip fee">
                <span class="price-val">${feeText}</span>
                <span class="price-sub">Borrow Fee</span>
              </div>
              <div class="price-chip deposit" style="border-color:rgba(16,185,129,0.3); background:rgba(16,185,129,0.08);">
                <span class="price-val" style="color:#10b981;">₹${sellPrice.toFixed(0)}</span>
                <span class="price-sub">Buy Permanently</span>
              </div>
            ` : `
              <div class="price-chip fee">
                <span class="price-val">${feeText}</span>
                <span class="price-sub">Borrow Fee</span>
              </div>
              <div class="price-chip deposit">
                <span class="price-val">${depositText}</span>
                <span class="price-sub">Refundable Deposit</span>
              </div>
            `}
          </div>

          <div style="display:flex; justify-content:space-between; align-items:center; margin-bottom:12px; font-size:0.75rem; color:#94a3b8;">
            <div class="max-duration-chip">
              ${isSellOnly ? `
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M6 2L3 6v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V6l-3-4z"/><line x1="3" y1="6" x2="21" y2="6"/><path d="M16 10a4 4 0 0 1-8 0"/></svg>
                <span>Permanent Ownership</span>
              ` : `
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><circle cx="12" cy="12" r="10"/><polyline points="12 6 12 12 16 14"/></svg>
                <span>Max ${item.maxDays} days</span>
              `}
            </div>
            <span style="font-size:0.75rem; color:#94a3b8;">Owner: <strong style="color:var(--text-main);">${escapeHtml(item.ownerName.split(' ')[0])}</strong> (${escapeHtml(item.ownerRollNo)})</span>
          </div>

          <div class="card-actions">
            ${isSold ? `
              <button class="btn btn-secondary" style="flex:1; opacity:0.65; cursor:not-allowed;" disabled>Sold Out</button>
            ` : isSellOnly ? `
              <button class="btn-buy-now" style="flex:1;" onclick="openBuyModal('${item.id}')">
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M6 2L3 6v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V6l-3-4z"/><line x1="3" y1="6" x2="21" y2="6"/><path d="M16 10a4 4 0 0 1-8 0"/></svg>
                <span>Buy (₹${sellPrice.toFixed(0)})</span>
              </button>
            ` : isBoth ? `
              <button class="btn-borrow" style="flex:1;" onclick="handleBorrow('${item.id}')">Borrow</button>
              <button class="btn-buy-now" style="flex:1; font-size:0.8rem; padding:6px 10px;" onclick="openBuyModal('${item.id}')" title="Buy outright">Buy ₹${sellPrice.toFixed(0)}</button>
            ` : isAvail ? `
              <button class="btn-borrow" style="flex:1;" onclick="handleBorrow('${item.id}')">Borrow Now</button>
            ` : `
              <button class="btn-waitlist" style="flex:1;" onclick="handleBorrow('${item.id}')">Waitlist (${waitCount})</button>
            `}
            <button class="btn-card-details" onclick="openItemDetails('${item.id}')" title="Inspect Item">Details</button>
          </div>
        </div>
      </div>
    `;
  }).join('');
}

function renderRequests() {
  let list = [...allRequests];

  if (currentRequestFilter === 'open') {
    list = list.filter(r => !r.isFulfilled);
  } else if (currentRequestFilter === 'fulfilled') {
    list = list.filter(r => r.isFulfilled);
  }

  const counterEl = document.getElementById('requests-counter-text');
  const openCount = allRequests.filter(r => !r.isFulfilled).length;
  if (counterEl) {
    counterEl.textContent = `${openCount} active open ${openCount === 1 ? 'need' : 'needs'} (${allRequests.length} total on board)`;
  }

  if (list.length === 0) {
    requestsGrid.innerHTML = `
      <div style="grid-column: 1 / -1; text-align: center; padding: 48px 20px;">
        <h3 style="margin-bottom: 8px; color: #94a3b8;">No ${currentRequestFilter === 'fulfilled' ? 'fulfilled' : 'open'} requests found</h3>
        <p style="color: #64748b; font-size: 0.9rem;">${currentRequestFilter === 'open' ? 'All student requests have been fulfilled! Have a requirement? Click "+ Post New Request".' : 'Post a wanted request to ask the community for resources.'}</p>
      </div>
    `;
    return;
  }

  requestsGrid.innerHTML = list.map(req => {
    const isOwner = activeStudent && activeStudent.rollNo === req.requesterRollNo;
    const catMeta = getCategoryMeta(req.category);

    return `
      <div class="request-card ${req.isFulfilled ? 'fulfilled' : 'open'}" data-id="${req.requestId}">
        <div class="req-header">
          <div style="display:flex; align-items:center; gap:8px;">
            <div class="cat-icon-mini ${catMeta.themeClass}">
              ${catMeta.iconSvg}
            </div>
            <span class="card-badge-category" style="position:static;">${escapeHtml(req.category)}</span>
          </div>
          <div style="display:flex; align-items:center; gap:6px;">
            ${req.isFulfilled 
              ? `<span class="req-badge-fulfilled">✓ Fulfilled</span>`
              : `<span class="req-badge-open">⚡ Active</span>`
            }
            <span class="req-urgency ${req.urgency.toLowerCase().includes('urgent') ? 'urgent' : ''}">${escapeHtml(req.urgency)}</span>
          </div>
        </div>

        <h3 class="req-title">${escapeHtml(req.itemTitle)}</h3>
        <p class="req-desc">${escapeHtml(req.description)}</p>

        <div class="req-footer">
          <div class="req-user">
            <div>
              <strong>${escapeHtml(req.requesterName)}</strong> 
              <span style="font-size:0.75rem; color:#94a3b8;">(${escapeHtml(req.requesterRollNo)})</span>
              ${isOwner ? '<span class="user-chip-you">You</span>' : ''}
            </div>
            <div style="font-size:0.75rem; color:#64748b; margin-top:2px;">Posted: ${escapeHtml(req.datePosted || 'Recent')}</div>
          </div>

          <div class="req-actions">
            ${req.isFulfilled ? `
              <span class="btn-fulfilled-status">
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><polyline points="20 6 9 17 4 12"/></svg>
                Fulfilled
              </span>
            ` : isOwner ? `
              <button class="btn btn-danger" style="font-size:0.78rem; padding:6px 12px;" onclick="handleCancelRequest('${req.requestId}')">
                Cancel Request
              </button>
            ` : `
              <button class="btn btn-primary" style="font-size:0.8rem; padding:7px 14px; gap:6px;" onclick="openFulfillModal('${req.requestId}')">
                <svg width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M17 21v-2a4 4 0 0 0-4-4H5a4 4 0 0 0-4 4v2"></path><circle cx="9" cy="7" r="4"></circle></svg>
                <span>I Have This!</span>
              </button>
            `}
          </div>
        </div>
      </div>
    `;
  }).join('');
}

function renderRecentlyViewed(items) {
  if (!items || items.length === 0) {
    recentGrid.innerHTML = `
      <div style="grid-column: 1 / -1; text-align: center; padding: 40px;">
        <h3 style="color:#94a3b8; margin-bottom:8px;">No Recently Viewed Items Yet</h3>
        <p style="color:#64748b; font-size:0.9rem;">Click "Details" on items in the Store Catalog to push them onto your LIFO view stack!</p>
      </div>
    `;
    return;
  }

  recentGrid.innerHTML = items.map((item, index) => {
    const isAvail = item.isAvailable;
    const catMeta = getCategoryMeta(item.category);
    return `
      <div class="item-card" data-id="${item.id}">
        <div class="card-category-banner">
          <div style="display:flex; align-items:center; gap:10px;">
            <div class="cat-icon-wrap ${catMeta.themeClass}">
              ${catMeta.iconSvg}
            </div>
            <div>
              <span class="card-badge-category">${escapeHtml(item.category)}</span>
              <span style="background:rgba(99,102,241,0.2); color:#a5b4fc; border:1px solid rgba(99,102,241,0.3); font-family:var(--font-mono); font-size:0.7rem; font-weight:700; padding:2px 6px; border-radius:4px; margin-left:6px;">
                ${index === 0 ? 'TOP OF STACK' : `#${index + 1} in Stack`}
              </span>
            </div>
          </div>
          <span class="card-badge-status ${isAvail ? 'badge-available' : 'badge-borrowed'}">
            ${isAvail ? '● Available' : '● Borrowed'}
          </span>
        </div>
        <div class="card-body">
          <span class="card-condition-pill">${escapeHtml(item.condition)}</span>
          <h3 class="card-title">${escapeHtml(item.title)}</h3>
          <p class="card-desc">${escapeHtml(item.description)}</p>
          <div class="card-actions" style="margin-top:auto;">
            <button class="btn-borrow" onclick="openItemDetails('${item.id}')">View Details</button>
          </div>
        </div>
      </div>
    `;
  }).join('');
}

// ---------------------------------------------------------------------------
// Actions: Borrow, Return, Waitlist, Lend, Request, Undo
// ---------------------------------------------------------------------------
window.handleBorrow = async function(itemId) {
  if (!activeStudent) {
    showToast('Please select an active student first', 'warn');
    return;
  }

  try {
    const res = await fetch(`${API_BASE}/api/items/borrow`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        itemId: itemId,
        studentRollNo: activeStudent.rollNo
      })
    });

    const result = await res.json();
    if (result.status === 'BORROWED') {
      showToast(result.message, 'success');
      // Update local student object
      if (!activeStudent.borrowedItemIds) activeStudent.borrowedItemIds = [];
      activeStudent.borrowedItemIds.push(itemId);
      updateActiveStudentUI();
    } else if (result.status === 'WAITLISTED') {
      showToast(`Joined FIFO Queue! Position #${result.queuePosition}`, 'info');
    } else {
      showToast(result.message || 'Action could not be completed', 'error');
    }

    // Refresh data
    await loadCatalogItems();
    await loadStats();
    if (itemModal.classList.contains('active')) {
      openItemDetails(itemId);
    }
  } catch (err) {
    console.error('Borrow error:', err);
    showToast('Network error during borrow action', 'error');
  }
};

function updateStarRatingUI(val) {
  currentFeedbackRating = val;
  const stars = document.querySelectorAll('#star-rating-wrap .star-btn');
  stars.forEach(btn => {
    const starVal = parseInt(btn.getAttribute('data-value'));
    if (starVal <= val) {
      btn.classList.add('active');
    } else {
      btn.classList.remove('active');
    }
  });
  const descEl = document.getElementById('star-rating-desc');
  if (descEl) {
    descEl.textContent = RATING_DESCRIPTIONS[val] || '';
  }
}

window.handleReturn = function(itemId) {
  if (!activeStudent) {
    showToast('Please sign in first', 'warn');
    authModal.classList.add('active');
    return;
  }

  const item = allItems.find(i => i.id === itemId);
  if (!item) return;

  currentReturningItemId = itemId;
  currentFeedbackRating = 5;

  document.getElementById('feedback-item-id').value = itemId;
  document.getElementById('feedback-comment').value = '';
  document.getElementById('feedback-item-subtitle').textContent = `Returning: ${item.title}`;

  const summary = document.getElementById('feedback-item-summary');
  const catMeta = getCategoryMeta(item.category);
  summary.innerHTML = `
    <div class="cat-icon-wrap ${catMeta.themeClass}" style="width:44px; height:44px; min-width:44px;">
      ${catMeta.iconSvg}
    </div>
    <div style="flex:1;">
      <div class="feedback-item-title">${escapeHtml(item.title)}</div>
      <div class="feedback-item-meta">Category: ${escapeHtml(item.category)} • Lender: <strong>${escapeHtml(item.ownerName)}</strong> (${escapeHtml(item.ownerRollNo)})</div>
    </div>
  `;

  updateStarRatingUI(5);
  feedbackModal.classList.add('active');
};

async function submitReturnWithFeedback(rating, comment) {
  if (!currentReturningItemId) return;
  const itemId = currentReturningItemId;

  try {
    const res = await fetch(`${API_BASE}/api/items/return`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        itemId: itemId,
        rating: rating || 5,
        comment: comment || '',
        reviewerRollNo: activeStudent ? activeStudent.rollNo : '',
        reviewerName: activeStudent ? activeStudent.name : ''
      })
    });

    const result = await res.json();
    feedbackModal.classList.remove('active');
    if (itemModal && itemModal.classList.contains('active')) {
      itemModal.classList.remove('active');
    }

    if (result.status === 'RETURNED_AVAILABLE') {
      showToast(result.message || 'Item returned successfully! Review saved.', 'success');
    } else if (result.status === 'AUTO_REASSIGNED') {
      showToast(`Item returned & review saved! ${result.message}`, 'info');
    } else {
      showToast(result.message || 'Return completed', 'info');
    }

    // Refresh active student profile & catalog
    await refreshActiveStudentProfile();
    await loadCatalogItems();
    await loadStats();
    updateDrawerContent();
  } catch (err) {
    console.error('Return error:', err);
    showToast('Network error during return action', 'error');
  }
}

window.handleCancelLending = function(itemId) {
  if (!activeStudent) {
    showToast('Please sign in first', 'warn');
    authModal.classList.add('active');
    return;
  }

  const item = allItems.find(i => i.id === itemId);
  const title = item ? item.title : itemId;

  pendingCancelItemId = itemId;
  document.getElementById('confirm-modal-title').textContent = 'Cancel Item Listing?';
  document.getElementById('confirm-modal-msg').innerHTML = `Are you sure you want to cancel and remove <strong>"${escapeHtml(title)}"</strong> from the store catalog?`;
  confirmModal.classList.add('active');
};

window.openItemDetails = async function(itemId) {
  const item = allItems.find(i => i.id === itemId);
  if (!item) return;

  // Record this view onto the student's LIFO stack
  if (activeStudent) {
    fetch(`${API_BASE}/api/student/view`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        studentRollNo: activeStudent.rollNo,
        itemId: itemId
      })
    }).catch(err => console.error(err));
  }

  const waitCount = item.waitlistCount || (item.waitlist ? item.waitlist.length : 0);
  const catMeta = getCategoryMeta(item.category);

  const isOwner = activeStudent && activeStudent.rollNo === item.ownerRollNo;
  const borrowPrice = item.borrowPrice || 0;
  const deposit = item.depositAmount || 0;
  const totalUpfront = borrowPrice + deposit;

  const modalBody = document.getElementById('item-modal-body');
  modalBody.innerHTML = `
    <div style="display:flex; flex-direction:column; gap:18px;">
      <!-- Category Banner Header (Clean, no photo images) -->
      <div class="details-category-banner ${catMeta.themeClass}">
        <div class="details-banner-left">
          <div class="details-cat-icon">
            ${catMeta.iconSvg}
          </div>
          <div>
            <div style="display:flex; align-items:center; gap:8px;">
              <span class="card-badge-category" style="position:static;">${escapeHtml(item.category)}</span>
              <span class="card-item-id">${escapeHtml(item.id)}</span>
            </div>
            <h2 style="font-size:1.35rem; font-weight:800; color:#f8fafc; margin-top:4px;">${escapeHtml(item.title)}</h2>
          </div>
        </div>
        <span class="card-badge-status ${item.isAvailable ? 'badge-available' : 'badge-borrowed'}" style="position:static;">
          ${item.isAvailable ? '● Available' : `● Borrowed (${waitCount} in waitlist)`}
        </span>
      </div>

      <div>
        <div style="display:flex; align-items:center; gap:8px; margin-bottom:10px;">
          <span class="card-condition-pill" style="margin-bottom:0;">Condition: ${escapeHtml(item.condition)}</span>
          <div class="card-rating-chip" style="margin-bottom:0;">
            ★ ${item.reviewCount > 0 ? `${item.averageRating.toFixed(1)} (${item.reviewCount} reviews)` : 'New item'}
          </div>
        </div>
        <p style="color:#94a3b8; font-size:0.92rem; line-height:1.6; margin-bottom:16px;">${escapeHtml(item.description)}</p>

        <!-- Pricing Breakdown -->
        ${item.isSold ? `
          <div style="background:rgba(100,116,139,0.12); border:1px solid rgba(100,116,139,0.25); border-radius:12px; padding:16px; margin-bottom:16px;">
            <div style="display:flex; align-items:center; gap:8px;">
              <span class="card-badge-sold">✓ PERMANENTLY SOLD</span>
              <span style="font-size:0.85rem; color:#94a3b8;">Sale completed</span>
            </div>
            <p style="color:var(--text-muted); font-size:0.88rem; margin-top:6px; line-height:1.5;">
              This resource was purchased permanently by PICT student <strong>${escapeHtml(item.buyerName || item.buyerRollNo)}</strong> (Roll: ${escapeHtml(item.buyerRollNo)}).
            </p>
          </div>
        ` : item.listingType === 'SELL' ? `
          <div class="pricing-breakdown-card" style="border-color:rgba(16,185,129,0.3); background:rgba(16,185,129,0.06);">
            <div class="pricing-box">
              <span class="pricing-box-label">Permanent Sale Price</span>
              <span class="pricing-box-val fee" style="color:#10b981;">₹${(item.sellPrice || 0).toFixed(0)}</span>
              <span class="pricing-box-hint">One-time purchase</span>
            </div>
            <div class="pricing-box" style="border-left: 1px solid rgba(255,255,255,0.08); padding-left: 12px;">
              <span class="pricing-box-label">Handover Type</span>
              <span class="pricing-box-val total" style="color:#10b981;">Direct Campus Handover</span>
              <span class="pricing-box-hint">UPI / Cash at PICT</span>
            </div>
          </div>
        ` : item.listingType === 'BOTH' ? `
          <div class="pricing-breakdown-card" style="margin-bottom:10px;">
            <div class="pricing-box">
              <span class="pricing-box-label">Borrow Fee</span>
              <span class="pricing-box-val fee">${borrowPrice === 0 ? 'FREE' : `₹${borrowPrice.toFixed(0)}`}</span>
              <span class="pricing-box-hint">Temporary rental</span>
            </div>
            <div class="pricing-box">
              <span class="pricing-box-label">Refundable Deposit</span>
              <span class="pricing-box-val deposit">${deposit === 0 ? 'FREE' : `₹${deposit.toFixed(0)}`}</span>
              <span class="pricing-box-hint">100% Refundable</span>
            </div>
          </div>
          <div class="pricing-breakdown-card" style="border-color:rgba(16,185,129,0.3); background:rgba(16,185,129,0.06);">
            <div class="pricing-box">
              <span class="pricing-box-label">Permanent Buyout Price</span>
              <span class="pricing-box-val fee" style="color:#10b981;">₹${(item.sellPrice || 0).toFixed(0)}</span>
              <span class="pricing-box-hint">Yours to keep permanently</span>
            </div>
          </div>
        ` : `
          <div class="pricing-breakdown-card">
            <div class="pricing-box">
              <span class="pricing-box-label">Borrow Fee</span>
              <span class="pricing-box-val fee">${borrowPrice === 0 ? 'FREE' : `₹${borrowPrice.toFixed(0)}`}</span>
              <span class="pricing-box-hint">Rental cost</span>
            </div>
            <div class="pricing-box">
              <span class="pricing-box-label">Security Deposit</span>
              <span class="pricing-box-val deposit">${deposit === 0 ? 'FREE' : `₹${deposit.toFixed(0)}`}</span>
              <span class="pricing-box-hint">100% Refundable</span>
            </div>
            <div class="pricing-box" style="border-left: 1px solid rgba(255,255,255,0.08); padding-left: 12px;">
              <span class="pricing-box-label">Total Upfront</span>
              <span class="pricing-box-val total">₹${totalUpfront.toFixed(0)}</span>
              <span class="pricing-box-hint">Payable at handover</span>
            </div>
          </div>
        `}

        ${isOwner ? `
          <div style="background: rgba(99, 102, 241, 0.1); border: 1px dashed rgba(99, 102, 241, 0.4); border-radius: 10px; padding: 12px 16px; margin-bottom: 16px; display: flex; align-items: center; justify-content: space-between; gap: 12px;">
            <div style="font-size: 0.85rem; color: #c7d2fe;">
              <strong>👑 Your Listing (${item.listingType === 'SELL' ? 'Permanent Sale' : item.listingType === 'BOTH' ? 'Rent or Sell' : 'Lender'})</strong>
              <div style="font-size: 0.75rem; color: #94a3b8;">${item.isSold ? `Sold to Roll: ${item.buyerRollNo}` : item.isAvailable ? 'Listed in store catalog.' : 'Currently borrowed by a student.'}</div>
            </div>
            ${item.isSold ? `
              <span style="font-size: 0.78rem; color: #10b981; font-weight: 700;">✓ Sold Out</span>
            ` : item.isAvailable ? `
              <button class="btn btn-danger" style="font-size: 0.8rem; padding: 6px 14px;" onclick="handleCancelLending('${item.id}')">
                Cancel / Delist Item
              </button>
            ` : `
              <span style="font-size: 0.75rem; color: #f59e0b; font-weight: 600;">(Cannot cancel while borrowed)</span>
            `}
          </div>
        ` : ''}

        <div style="background:rgba(30,41,59,0.7); border:1px solid rgba(255,255,255,0.08); border-radius:12px; padding:16px; margin-bottom:16px; display:grid; grid-template-columns:1fr 1fr; gap:12px;">
          <div>
            <span style="font-size:0.75rem; color:#64748b;">Lender / Owner</span>
            <div style="font-weight:700; color:var(--text-main);">${escapeHtml(item.ownerName)}</div>
            <div style="font-size:0.78rem; color:var(--text-muted);">Roll: ${escapeHtml(item.ownerRollNo)} • ${escapeHtml(item.ownerContact)}</div>
          </div>
          <div>
            <span style="font-size:0.75rem; color:#64748b;">Offer Type</span>
            <div style="font-weight:700; color:#38bdf8;">${item.listingType === 'SELL' ? 'Permanent Sale' : item.listingType === 'BOTH' ? 'Borrow or Buy' : `Max ${item.maxDays} days`}</div>
            <div style="font-size:0.78rem; color:var(--text-muted);">${item.listingType === 'SELL' ? 'Keep forever' : 'SPPU Semester friendly'}</div>
          </div>
          ${(!item.isAvailable && !item.isSold) ? `
            <div style="grid-column: 1 / -1; border-top:1px solid rgba(255,255,255,0.06); padding-top:8px;">
              <span style="font-size:0.75rem; color:#f59e0b; font-weight:600;">Currently Borrowed By:</span>
              <div style="font-size:0.85rem; color:#cbd5e1;">${escapeHtml(item.currentBorrowerName || item.currentBorrowerRollNo)} (Roll: ${escapeHtml(item.currentBorrowerRollNo)})</div>
            </div>
          ` : ''}
        </div>

        <!-- Verified Reviews Section -->
        <div class="item-reviews-wrap">
          <div class="reviews-header">
            <div class="reviews-title">
              <span>Verified Borrower Feedback</span>
            </div>
            <div class="reviews-badge">
              ★ ${item.reviewCount > 0 ? `${item.averageRating.toFixed(1)} / 5.0 (${item.reviewCount})` : 'No reviews yet'}
            </div>
          </div>
          <div class="reviews-list">
            ${(item.feedbacks && item.feedbacks.length > 0)
              ? item.feedbacks.map(f => {
                  const starStr = '★'.repeat(Math.max(1, Math.min(5, f.rating))) + '☆'.repeat(Math.max(0, 5 - f.rating));
                  return `
                    <div class="review-item">
                      <div class="review-top">
                        <div>
                          <span class="review-author">${escapeHtml(f.reviewerName || 'PICT Student')}</span>
                          <span style="font-size:0.75rem; color:#64748b;">(${escapeHtml(f.reviewerRollNo || 'Peer')})</span>
                        </div>
                        <div>
                          <span class="review-stars">${starStr}</span>
                          <span class="review-date">${escapeHtml(f.timestamp || '')}</span>
                        </div>
                      </div>
                      <div class="review-comment">${escapeHtml(f.comment || 'Verified return with positive rating.')}</div>
                    </div>
                  `;
                }).join('')
              : `
                <div style="text-align: center; padding: 12px; color: #64748b; font-size: 0.85rem;">
                  No reviews yet for this resource. Be the first to borrow and leave feedback!
                </div>
              `
            }
          </div>
        </div>

        <div style="display:flex; gap:12px; flex-wrap:wrap;">
          ${isOwner ? `
            ${item.isSold ? `
              <span style="font-size:0.9rem; color:#10b981; font-weight:700; padding:8px;">✓ Item Sold to ${escapeHtml(item.buyerName || item.buyerRollNo)}</span>
            ` : item.isAvailable ? `
              <button class="btn btn-danger" style="flex:1;" onclick="handleCancelLending('${item.id}')">
                Cancel My Listing
              </button>
            ` : `
              <button class="btn btn-secondary" style="flex:1;" onclick="handleReturn('${item.id}')">
                Mark as Returned to Me
              </button>
            `}
          ` : `
            ${item.isSold ? `
              <button class="btn btn-secondary" style="flex:1; opacity:0.65;" disabled>Sold to Another Student</button>
            ` : item.listingType === 'SELL' ? `
              <button class="btn-buy-now" style="flex:1; padding:12px;" onclick="openBuyModal('${item.id}')">
                <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2.5"><path d="M6 2L3 6v14a2 2 0 0 0 2 2h14a2 2 0 0 0 2-2V6l-3-4z"/><line x1="3" y1="6" x2="21" y2="6"/><path d="M16 10a4 4 0 0 1-8 0"/></svg>
                <span>Buy Permanently (₹${(item.sellPrice || 0).toFixed(0)})</span>
              </button>
            ` : item.listingType === 'BOTH' ? `
              <button class="btn btn-primary" style="flex:1;" onclick="handleBorrow('${item.id}')">Borrow Resource</button>
              <button class="btn-buy-now" style="flex:1; padding:10px 14px;" onclick="openBuyModal('${item.id}')">Buy Outright (₹${(item.sellPrice || 0).toFixed(0)})</button>
            ` : item.isAvailable ? `
              <button class="btn btn-primary" style="flex:1;" onclick="handleBorrow('${item.id}')">Borrow This Resource</button>
            ` : `
              <button class="btn-waitlist" style="flex:1;" onclick="handleBorrow('${item.id}')">Join FIFO Waiting Queue (${waitCount} waiting)</button>
            `}
            ${(!item.isAvailable && !item.isSold && activeStudent && activeStudent.rollNo === item.currentBorrowerRollNo)
              ? `<button class="btn btn-secondary" onclick="handleReturn('${item.id}')">Return Resource</button>`
              : ''
            }
          `}
        </div>
      </div>
    </div>
  `;

  itemModal.classList.add('active');
};

// ---------------------------------------------------------------------------
// Buy Resource Modal & Handover Confirmation
// ---------------------------------------------------------------------------
window.openBuyModal = function(itemId) {
  if (!activeStudent) {
    showToast('Please sign in first to purchase an item', 'warn');
    authModal.classList.add('active');
    return;
  }

  const item = allItems.find(i => i.id === itemId);
  if (!item) return;

  if (activeStudent.rollNo === item.ownerRollNo) {
    showToast('You cannot purchase your own listed item!', 'warn');
    return;
  }

  if (item.isSold) {
    showToast('Sorry, this item has already been sold', 'error');
    return;
  }

  const catMeta = getCategoryMeta(item.category);
  const cleanPhone = (item.ownerContact || '').replace(/[^0-9]/g, '');
  const waLink = cleanPhone ? `https://wa.me/${cleanPhone}?text=${encodeURIComponent(`Hi ${item.ownerName}, I would like to buy your "${item.title}" listed on PICT Exchange for ₹${item.sellPrice}. Are you free to meet on campus?`)}` : '#';

  const modalBody = document.getElementById('buy-modal-body');
  modalBody.innerHTML = `
    <div style="display:flex; flex-direction:column; gap:16px;">
      <div class="fulfill-item-summary">
        <div class="cat-icon-wrap ${catMeta.themeClass}" style="width:48px; height:48px; min-width:48px;">
          ${catMeta.iconSvg}
        </div>
        <div style="flex:1;">
          <h4 style="font-size:1.05rem; font-weight:800; color:var(--text-main);">${escapeHtml(item.title)}</h4>
          <div style="font-size:0.8rem; color:var(--text-muted); margin-top:2px;">
            Category: <strong>${escapeHtml(item.category)}</strong> • Condition: <strong>${escapeHtml(item.condition)}</strong>
          </div>
        </div>
      </div>

      <!-- Pricing Summary Card -->
      <div style="background:rgba(16,185,129,0.08); border:1.5px solid rgba(16,185,129,0.3); border-radius:12px; padding:16px; display:flex; justify-content:space-between; align-items:center;">
        <div>
          <span style="font-size:0.75rem; color:#10b981; font-weight:700; text-transform:uppercase;">Agreed Purchase Price</span>
          <div style="font-size:1.6rem; font-weight:900; color:#10b981;">₹${(item.sellPrice || 0).toFixed(0)}</div>
        </div>
        <div style="text-align:right;">
          <span class="card-badge-sale">Permanent Ownership</span>
          <div style="font-size:0.75rem; color:var(--text-muted); margin-top:4px;">No deposit • Yours to keep</div>
        </div>
      </div>

      <!-- Seller Details Card -->
      <div style="background:var(--bg-surface); border:1px solid var(--border-subtle); border-radius:10px; padding:16px;">
        <span style="font-size:0.72rem; color:var(--text-faint); text-transform:uppercase; font-weight:700; letter-spacing:0.04em;">Lender / Seller Details</span>
        <div style="font-size:1.05rem; font-weight:800; color:var(--text-main); margin-top:4px;">${escapeHtml(item.ownerName)}</div>
        <div style="font-size:0.82rem; color:var(--text-muted); margin-top:2px;">PICT Roll Number: <strong style="color:var(--text-main);">${escapeHtml(item.ownerRollNo)}</strong></div>
        
        <div style="display:flex; gap:10px; margin-top:14px; flex-wrap:wrap;">
          ${cleanPhone ? `
            <a href="${waLink}" target="_blank" rel="noopener noreferrer" class="btn btn-whatsapp" style="flex:1; text-decoration:none; display:inline-flex; align-items:center; justify-content:center; gap:6px; font-size:0.84rem; padding:10px 14px; background:#25D366; color:#fff; border-radius:8px; font-weight:700;">
              <span>WhatsApp Seller</span>
            </a>
            <a href="tel:${escapeHtml(item.ownerContact)}" class="btn btn-secondary" style="flex:1; text-decoration:none; display:inline-flex; align-items:center; justify-content:center; gap:6px; font-size:0.84rem; padding:10px 14px;">
              <span>Call (${escapeHtml(item.ownerContact)})</span>
            </a>
          ` : `
            <span style="font-size:0.85rem; color:var(--text-muted);">Contact: ${escapeHtml(item.ownerContact || 'Available on campus')}</span>
          `}
        </div>
      </div>

      <div style="background:rgba(59,130,246,0.08); border:1px dashed rgba(59,130,246,0.3); border-radius:10px; padding:12px; font-size:0.82rem; color:#93c5fd; line-height:1.4;">
        🤝 <strong>Campus Handover Notice:</strong> When you confirm purchase, this item is reserved as <strong>SOLD</strong> to you on the exchange. Please meet ${escapeHtml(item.ownerName)} at the PICT Canteen / Reading Hall and pay ₹${(item.sellPrice || 0).toFixed(0)} via UPI or Cash.
      </div>

      <div style="display:flex; gap:10px; margin-top:8px;">
        <button type="button" class="btn btn-secondary" style="flex:1;" onclick="document.getElementById('buy-modal-overlay').classList.remove('active')">Cancel</button>
        <button type="button" class="btn-buy-now" style="flex:1.4; padding:10px 16px; font-size:0.92rem;" onclick="confirmBuyItem('${item.id}')">
          ✓ Confirm Purchase (₹${(item.sellPrice || 0).toFixed(0)})
        </button>
      </div>
    </div>
  `;

  if (itemModal) itemModal.classList.remove('active');
  buyModal.classList.add('active');
};

window.confirmBuyItem = async function(itemId) {
  if (!activeStudent) {
    showToast('Please sign in first', 'warn');
    return;
  }

  try {
    const res = await fetch(`${API_BASE}/api/items/buy`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        itemId: itemId,
        studentRollNo: activeStudent.rollNo
      })
    });

    const result = await res.json();
    buyModal.classList.remove('active');

    if (result.status === 'BOUGHT') {
      showToast(result.message || 'Item purchased successfully!', 'success');
      await refreshActiveStudentProfile();
      await loadCatalogItems();
      await loadStats();
      updateDrawerContent();
    } else {
      showToast(result.message || 'Could not complete purchase', 'error');
    }
  } catch (err) {
    console.error('Buy error:', err);
    showToast('Network error while purchasing item', 'error');
  }
};

window.openFulfillModal = function(reqId) {
  if (!activeStudent) {
    showToast('Please sign in first to respond to a request', 'warn');
    authModal.classList.add('active');
    return;
  }

  const req = allRequests.find(r => r.requestId === reqId);
  if (!req) return;

  const catMeta = getCategoryMeta(req.category);
  const cleanPhone = (req.requesterContact || '').replace(/[^0-9]/g, '');
  const waLink = cleanPhone ? `https://wa.me/${cleanPhone}?text=${encodeURIComponent(`Hi ${req.requesterName}, I saw your request on PICT Exchange for "${req.itemTitle}". I have it and can lend it to you!`)}` : '#';

  const modalBody = document.getElementById('fulfill-modal-body');
  modalBody.innerHTML = `
    <div style="display:flex; flex-direction:column; gap:16px;">
      <div class="fulfill-item-summary">
        <div class="cat-icon-wrap ${catMeta.themeClass}" style="width:44px; height:44px;">
          ${catMeta.iconSvg}
        </div>
        <div style="flex:1;">
          <div style="font-size:0.75rem; color:#38bdf8; font-weight:700;">WANTED RESOURCE</div>
          <h4 style="font-size:1.1rem; font-weight:800; color:#f8fafc; margin-top:2px;">${escapeHtml(req.itemTitle)}</h4>
          <div style="font-size:0.82rem; color:#94a3b8; margin-top:4px;">${escapeHtml(req.description)}</div>
          <div style="margin-top:6px;">
            <span class="req-urgency ${req.urgency.toLowerCase().includes('urgent') ? 'urgent' : ''}">${escapeHtml(req.urgency)}</span>
          </div>
        </div>
      </div>

      <div style="background:rgba(30,41,59,0.7); border:1px solid rgba(255,255,255,0.08); border-radius:10px; padding:16px;">
        <span style="font-size:0.72rem; color:#64748b; text-transform:uppercase; font-weight:700; letter-spacing:0.04em;">Requester Student</span>
        <div style="font-size:1rem; font-weight:800; color:#e2e8f0; margin-top:4px;">${escapeHtml(req.requesterName)}</div>
        <div style="font-size:0.8rem; color:#94a3b8; margin-top:2px;">PICT Roll Number: <strong style="color:#f8fafc;">${escapeHtml(req.requesterRollNo)}</strong></div>
        
        <div style="display:flex; gap:10px; margin-top:14px;">
          ${cleanPhone ? `
            <a href="${waLink}" target="_blank" rel="noopener noreferrer" class="btn btn-whatsapp" style="flex:1; text-decoration:none; display:inline-flex; align-items:center; justify-content:center; gap:6px; font-size:0.84rem; padding:10px 14px; background:#25D366; color:#fff; border-radius:8px; font-weight:700;">
              <span>WhatsApp Peer</span>
            </a>
            <a href="tel:${escapeHtml(req.requesterContact)}" class="btn btn-secondary" style="flex:1; text-decoration:none; display:inline-flex; align-items:center; justify-content:center; gap:6px; font-size:0.84rem; padding:10px 14px;">
              <span>Call (${escapeHtml(req.requesterContact)})</span>
            </a>
          ` : `
            <span style="font-size:0.85rem; color:#94a3b8;">Contact: ${escapeHtml(req.requesterContact || 'Available in college')}</span>
          `}
        </div>
      </div>

      <div style="background:rgba(59,130,246,0.08); border:1px dashed rgba(59,130,246,0.3); border-radius:10px; padding:12px; font-size:0.82rem; color:#93c5fd; line-height:1.4;">
        🤝 <strong>Community Handover:</strong> Reach out to ${escapeHtml(req.requesterName)} to meet on campus. Once you hand over the item, mark it fulfilled below so fellow students know it has been arranged!
      </div>

      <div style="display:flex; gap:10px; margin-top:8px;">
        <button type="button" class="btn btn-secondary" style="flex:1;" onclick="document.getElementById('fulfill-modal-overlay').classList.remove('active')">Close</button>
        <button type="button" class="btn btn-primary" style="flex:1.4;" onclick="confirmFulfillRequest('${req.requestId}')">
          ✓ Mark as Fulfilled by Me
        </button>
      </div>
    </div>
  `;

  document.getElementById('fulfill-modal-overlay').classList.add('active');
};

window.confirmFulfillRequest = async function(reqId) {
  try {
    const res = await fetch(`${API_BASE}/api/requests/fulfill`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({ requestId: reqId })
    });
    const result = await res.json();
    document.getElementById('fulfill-modal-overlay').classList.remove('active');
    if (result.status === 'SUCCESS') {
      showToast('Thank you! Request marked as fulfilled. Connect with your peer!', 'success');
      await loadRequests();
      await loadStats();
    } else {
      showToast(result.message || 'Could not fulfill request', 'error');
    }
  } catch (err) {
    showToast('Error fulfilling request', 'error');
  }
};

window.handleCancelRequest = async function(reqId) {
  if (!activeStudent) return;
  try {
    const res = await fetch(`${API_BASE}/api/requests/delete`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' },
      body: JSON.stringify({
        requestId: reqId,
        requesterRollNo: activeStudent.rollNo
      })
    });
    const result = await res.json();
    if (result.status === 'SUCCESS') {
      showToast(result.message || 'Request cancelled', 'info');
      await loadRequests();
      await loadStats();
    } else {
      showToast(result.message || 'Could not cancel request', 'error');
    }
  } catch (err) {
    showToast('Failed to cancel request', 'error');
  }
};

async function handleUndo() {
  try {
    const res = await fetch(`${API_BASE}/api/undo`, {
      method: 'POST',
      headers: { 'Content-Type': 'application/json' }
    });
    const result = await res.json();
    if (result.status === 'SUCCESS') {
      showToast(result.message, 'info');
      await loadCatalogItems();
      await loadStats();
      await refreshActiveStudentProfile();
    } else {
      showToast(result.message || 'No action to undo', 'warn');
    }
  } catch (err) {
    showToast('Failed to execute undo', 'error');
  }
}

async function refreshActiveStudentProfile() {
  if (!activeStudent) return;
  try {
    const res = await fetch(`${API_BASE}/api/student?roll=${encodeURIComponent(activeStudent.rollNo)}`);
    if (res.ok) {
      activeStudent = await res.json();
      updateActiveStudentUI();
    }
  } catch (err) {
    console.error(err);
  }
}

// ---------------------------------------------------------------------------
// Drawer Functions
// ---------------------------------------------------------------------------
function openDashboardDrawer() {
  updateDrawerContent();
  dashboardDrawer.classList.add('active');
  drawerOverlay.classList.add('active');
}

function closeDashboardDrawer() {
  dashboardDrawer.classList.remove('active');
  drawerOverlay.classList.remove('active');
}

function updateDrawerContent() {
  if (!activeStudent) return;

  const borrowedList = document.getElementById('drawer-borrowed-list');
  const lentList = document.getElementById('drawer-lent-list');
  const purchasedList = document.getElementById('drawer-purchased-list');
  const borrowedCount = document.getElementById('drawer-borrowed-count');
  const lentCount = document.getElementById('drawer-lent-count');
  const purchasedCount = document.getElementById('drawer-purchased-count');

  const borrowedIds = activeStudent.borrowedItemIds || [];
  const lentIds = activeStudent.lentItemIds || [];
  const purchasedIds = activeStudent.purchasedItemIds || [];

  if (borrowedCount) borrowedCount.textContent = borrowedIds.length;
  if (lentCount) lentCount.textContent = lentIds.length;
  if (purchasedCount) purchasedCount.textContent = purchasedIds.length;

  // 1. Render borrowed items
  if (borrowedList) {
    if (borrowedIds.length === 0) {
      borrowedList.innerHTML = `<p class="empty-state">You have not borrowed any items.</p>`;
    } else {
      borrowedList.innerHTML = borrowedIds.map(id => {
        const item = allItems.find(i => i.id === id);
        const title = item ? item.title : `Item ${id}`;
        return `
          <div class="drawer-item-card">
            <div>
              <div class="drawer-item-title">${escapeHtml(title)}</div>
              <div class="drawer-item-meta">ID: ${escapeHtml(id)}</div>
            </div>
            <button class="btn btn-secondary" style="font-size:0.75rem; padding:4px 10px;" onclick="handleReturn('${id}')">Return</button>
          </div>
        `;
      }).join('');
    }
  }

  // 2. Render lent / listed items
  if (lentList) {
    if (lentIds.length === 0) {
      lentList.innerHTML = `<p class="empty-state">You have not listed any items for lending or sale.</p>`;
    } else {
      lentList.innerHTML = lentIds.map(id => {
        const item = allItems.find(i => i.id === id);
        const title = item ? item.title : `Item ${id}`;
        const isSold = item ? item.isSold : false;
        const isAvail = item ? item.isAvailable : true;
        const isSell = item ? (item.listingType === 'SELL') : false;
        const isBoth = item ? (item.listingType === 'BOTH') : false;

        let statusText = 'AVAILABLE';
        let statusColor = '#10b981';
        let subText = 'Available in Store';

        if (isSold) {
          statusText = 'SOLD';
          statusColor = '#10b981';
          subText = `Sold to Roll: ${item.buyerRollNo}`;
        } else if (!isAvail) {
          statusText = 'BORROWED';
          statusColor = '#f59e0b';
          subText = 'Currently borrowed by peer';
        } else if (isSell) {
          statusText = `FOR SALE (₹${(item.sellPrice || 0).toFixed(0)})`;
          statusColor = '#10b981';
          subText = 'Permanent listing';
        } else if (isBoth) {
          statusText = `RENT / SALE`;
          statusColor = '#a78bfa';
          subText = `Rent ₹${(item.borrowPrice || 0).toFixed(0)} / Sell ₹${(item.sellPrice || 0).toFixed(0)}`;
        }

        return `
          <div class="drawer-item-card" style="align-items: center;">
            <div style="flex: 1; min-width: 0; padding-right: 8px;">
              <div class="drawer-item-title" style="white-space: nowrap; overflow: hidden; text-overflow: ellipsis;">${escapeHtml(title)}</div>
              <div class="drawer-item-meta">${subText}</div>
            </div>
            <div style="display: flex; flex-direction: column; align-items: flex-end; gap: 4px;">
              <span style="font-size:0.72rem; color:${statusColor}; font-weight:700;">
                ${statusText}
              </span>
              ${(!isSold && isAvail) 
                ? `<button class="btn btn-danger" style="font-size:0.72rem; padding:3px 8px;" onclick="handleCancelLending('${id}')" title="Cancel listing and remove from catalog">Cancel Listing</button>`
                : isSold 
                  ? `<span style="font-size:0.68rem; color:#10b981; font-weight:600;">Completed</span>`
                  : `<span style="font-size:0.68rem; color:#94a3b8;" title="Cannot cancel while item is borrowed by a peer">In Use</span>`
              }
            </div>
          </div>
        `;
      }).join('');
    }
  }

  // 3. Render purchased items
  if (purchasedList) {
    if (purchasedIds.length === 0) {
      purchasedList.innerHTML = `<p class="empty-state">You have not bought any items yet.</p>`;
    } else {
      purchasedList.innerHTML = purchasedIds.map(id => {
        const item = allItems.find(i => i.id === id);
        const title = item ? item.title : `Item ${id}`;
        const price = item ? (item.sellPrice || 0) : 0;
        const seller = item ? item.ownerName : 'PICT Peer';
        return `
          <div class="drawer-item-card">
            <div>
              <div class="drawer-item-title">${escapeHtml(title)}</div>
              <div class="drawer-item-meta">Bought from: ${escapeHtml(seller)} • <strong>₹${price.toFixed(0)}</strong></div>
            </div>
            <span class="card-badge-sale" style="font-size:0.72rem;">Yours to Keep</span>
          </div>
        `;
      }).join('');
    }
  }
}

// ---------------------------------------------------------------------------
// Toast Helper
// ---------------------------------------------------------------------------
function showToast(message, type = 'info') {
  const container = document.getElementById('toast-container');
  const toast = document.createElement('div');
  toast.className = `toast ${type}`;
  toast.innerHTML = `
    <span>${escapeHtml(message)}</span>
  `;
  container.appendChild(toast);

  setTimeout(() => {
    toast.style.opacity = '0';
    toast.style.transform = 'translateY(10px)';
    setTimeout(() => toast.remove(), 300);
  }, 4000);
}

function escapeHtml(str) {
  if (!str) return '';
  return str.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/>/g, '&gt;').replace(/"/g, '&quot;');
}

// ---------------------------------------------------------------------------
// Event Listeners
// ---------------------------------------------------------------------------
function bindEvents() {
  // Search bar live filter
  let searchTimeout;
  searchInput.addEventListener('input', (e) => {
    clearTimeout(searchTimeout);
    currentSearch = e.target.value.trim();
    clearSearchBtn.style.display = currentSearch ? 'block' : 'none';
    searchTimeout = setTimeout(() => {
      loadCatalogItems();
    }, 250);
  });

  clearSearchBtn.addEventListener('click', () => {
    searchInput.value = '';
    currentSearch = '';
    clearSearchBtn.style.display = 'none';
    loadCatalogItems();
  });

  // Student Switcher
  activeStudentSelect.addEventListener('change', (e) => {
    const selected = allStudents.find(s => s.rollNo === e.target.value);
    if (selected) {
      activeStudent = selected;
      updateActiveStudentUI();
      showToast(`Switched active profile to ${activeStudent.name}`, 'info');
      // If currently on recent tab, refresh
      if (recentView.classList.contains('active')) {
        loadRecentlyViewed();
      }
    }
  });

  // Category filter pills
  categoryPills.addEventListener('click', (e) => {
    const btn = e.target.closest('.pill-btn');
    if (!btn) return;
    document.querySelectorAll('.pill-btn').forEach(b => b.classList.remove('active'));
    btn.classList.add('active');
    currentCategory = btn.dataset.category;
    loadCatalogItems();
  });

  // Listing Type filter pills (All, Rent, Sell)
  const listingTypePills = document.getElementById('listing-type-pills');
  if (listingTypePills) {
    listingTypePills.addEventListener('click', (e) => {
      const btn = e.target.closest('.type-filter-btn');
      if (!btn) return;
      document.querySelectorAll('.type-filter-btn').forEach(b => b.classList.remove('active'));
      btn.classList.add('active');
      currentListingTypeFilter = btn.dataset.type || 'all';
      renderCatalog();
    });
  }


  // Availability filter
  availabilityFilter.addEventListener('change', (e) => {
    currentAvailability = e.target.value;
    loadCatalogItems();
  });

  // Sort dropdown
  sortSelect.addEventListener('change', (e) => {
    currentSort = e.target.value;
    renderCatalog();
  });

  // View Switcher Tabs
  tabCatalog.addEventListener('click', () => switchView('catalog'));
  tabRequests.addEventListener('click', () => switchView('requests'));
  tabRecent.addEventListener('click', () => switchView('recent'));

  function switchView(viewName) {
    [tabCatalog, tabRequests, tabRecent].forEach(t => t.classList.remove('active'));
    [catalogView, requestsView, recentView].forEach(v => v.classList.remove('active'));

    if (viewName === 'catalog') {
      tabCatalog.classList.add('active');
      catalogView.classList.add('active');
    } else if (viewName === 'requests') {
      tabRequests.classList.add('active');
      requestsView.classList.add('active');
      loadRequests();
    } else if (viewName === 'recent') {
      tabRecent.classList.add('active');
      recentView.classList.add('active');
      loadRecentlyViewed();
    }
  }

  // Undo button
  document.getElementById('btn-undo-action').addEventListener('click', handleUndo);

  // Stats / DS Inspector
  document.getElementById('btn-view-stats').addEventListener('click', async () => {
    await loadStats();
    statsModal.classList.add('active');
  });
  document.getElementById('btn-close-stats-modal').addEventListener('click', () => {
    statsModal.classList.remove('active');
  });

  // Drawer buttons
  document.getElementById('btn-open-drawer').addEventListener('click', openDashboardDrawer);
  document.getElementById('btn-close-drawer').addEventListener('click', closeDashboardDrawer);
  drawerOverlay.addEventListener('click', closeDashboardDrawer);

  // Lend Modal
  document.getElementById('btn-open-lend-modal').addEventListener('click', () => {
    lendModal.classList.add('active');
  });
  document.getElementById('btn-close-lend-modal').addEventListener('click', () => {
    lendModal.classList.remove('active');
  });
  document.getElementById('btn-cancel-lend').addEventListener('click', () => {
    lendModal.classList.remove('active');
  });

  // Buy Modal Close
  if (btnCloseBuyModal) {
    btnCloseBuyModal.addEventListener('click', () => {
      if (buyModal) buyModal.classList.remove('active');
    });
  }

  // Lend Form Submit
  document.getElementById('lend-item-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    if (!activeStudent) {
      showToast('Please select a student profile first', 'warn');
      return;
    }

    const title = document.getElementById('lend-title').value.trim();
    const category = document.getElementById('lend-category').value;
    const condition = document.getElementById('lend-condition').value;
    const desc = document.getElementById('lend-desc').value.trim();
    const listingType = document.querySelector('input[name="listingIntent"]:checked')?.value || 'RENT';
    const price = parseFloat(document.getElementById('lend-price')?.value) || 0;
    const deposit = parseFloat(document.getElementById('lend-deposit')?.value) || 0;
    const days = parseInt(document.getElementById('lend-days')?.value) || 7;
    const sellPrice = parseFloat(document.getElementById('lend-sell-price')?.value) || 0;

    try {
      const res = await fetch(`${API_BASE}/api/items/lend`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          title,
          category,
          condition,
          description: desc,
          listingType,
          borrowPrice: price,
          depositAmount: deposit,
          maxDays: days,
          sellPrice,
          imageUrl: '',
          ownerRollNo: activeStudent.rollNo,
          ownerName: activeStudent.name,
          ownerContact: activeStudent.contactNo
        })
      });

      const result = await res.json();
      if (result.status === 'SUCCESS') {
        showToast(result.message, 'success');
        document.getElementById('lend-item-form').reset();
        handleIntentChange('RENT');
        lendModal.classList.remove('active');
        await refreshActiveStudentProfile();
        await loadCatalogItems();
        await loadStats();
      } else {
        showToast(result.message || 'Error listing item', 'error');
      }
    } catch (err) {
      showToast('Network error while listing item', 'error');
    }
  });

  // Request Modal Open & Close
  const handleOpenPostRequestModal = () => {
    if (!activeStudent) {
      showToast('Please sign in first to post a resource request', 'warn');
      authModal.classList.add('active');
      return;
    }
    requestModal.classList.add('active');
  };

  document.getElementById('btn-open-request-modal').addEventListener('click', handleOpenPostRequestModal);
  const btnBoardPost = document.getElementById('btn-board-post-request');
  if (btnBoardPost) {
    btnBoardPost.addEventListener('click', handleOpenPostRequestModal);
  }
  document.getElementById('btn-close-request-modal').addEventListener('click', () => {
    requestModal.classList.remove('active');
  });
  document.getElementById('btn-cancel-request').addEventListener('click', () => {
    requestModal.classList.remove('active');
  });

  // Request Form Submit
  document.getElementById('request-item-form').addEventListener('submit', async (e) => {
    e.preventDefault();
    if (!activeStudent) {
      showToast('Please select a student profile first', 'warn');
      return;
    }

    const title = document.getElementById('req-title').value.trim();
    const category = document.getElementById('req-category').value;
    const urgency = document.getElementById('req-urgency').value;
    const desc = document.getElementById('req-desc').value.trim();

    try {
      const res = await fetch(`${API_BASE}/api/requests/create`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          itemTitle: title,
          category,
          urgency,
          description: desc,
          requesterRollNo: activeStudent.rollNo,
          requesterName: activeStudent.name,
          requesterContact: activeStudent.contactNo
        })
      });

      const result = await res.json();
      if (result.status === 'SUCCESS') {
        showToast(result.message, 'success');
        document.getElementById('request-item-form').reset();
        requestModal.classList.remove('active');
        await loadRequests();
        await loadStats();
      } else {
        showToast(result.message || 'Error creating request', 'error');
      }
    } catch (err) {
      showToast('Network error while posting request', 'error');
    }
  });

  // Close Item Modal
  document.getElementById('btn-close-item-modal').addEventListener('click', () => {
    itemModal.classList.remove('active');
  });

  // Auth Modal controls
  if (btnOpenLoginModal) {
    btnOpenLoginModal.addEventListener('click', () => {
      authModal.classList.add('active');
    });
  }
  if (btnSwitchAccount) {
    btnSwitchAccount.addEventListener('click', () => {
      authModal.classList.add('active');
    });
  }
  if (navUserProfile) {
    navUserProfile.addEventListener('click', () => {
      authModal.classList.add('active');
    });
  }
  if (btnCloseAuthModal) {
    btnCloseAuthModal.addEventListener('click', () => {
      authModal.classList.remove('active');
    });
  }

  // Auth Tab Switcher
  tabAuthLogin.addEventListener('click', () => {
    tabAuthLogin.classList.add('active');
    tabAuthRegister.classList.remove('active');
    authContentLogin.classList.add('active');
    authContentRegister.classList.remove('active');
  });
  tabAuthRegister.addEventListener('click', () => {
    tabAuthRegister.classList.add('active');
    tabAuthLogin.classList.remove('active');
    authContentRegister.classList.add('active');
    authContentLogin.classList.remove('active');
  });

  // 1-Click Quick Login
  window.handleQuickLogin = async function(rollNo) {
    const student = allStudents.find(s => s.rollNo === rollNo);
    if (student) {
      setLoggedInStudent(student);
      authModal.classList.remove('active');
      showToast(`Signed in as ${student.name} (Roll: ${student.rollNo})`, 'success');
      await refreshActiveStudentProfile();
      await loadCatalogItems();
      await loadStats();
      if (dashboardDrawer.classList.contains('active')) {
        updateDrawerContent();
      }
    }
  };

  // Sign In Form Submit
  loginForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    const rollInput = document.getElementById('login-roll-input').value.trim();
    if (!rollInput) return;

    try {
      const res = await fetch(`${API_BASE}/api/student/login`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ rollNo: rollInput })
      });
      const result = await res.json();
      if (result.status === 'SUCCESS' && result.student) {
        // Also update local list if needed
        const existing = allStudents.find(s => s.rollNo === result.student.rollNo);
        if (!existing) allStudents.push(result.student);

        setLoggedInStudent(result.student);
        authModal.classList.remove('active');
        loginForm.reset();
        showToast(result.message || `Welcome back, ${result.student.name}!`, 'success');
        await loadCatalogItems();
        await loadStats();
        if (dashboardDrawer.classList.contains('active')) {
          updateDrawerContent();
        }
      } else {
        showToast(result.message || 'Student not found', 'error');
      }
    } catch (err) {
      showToast('Failed to sign in', 'error');
    }
  });

  // Register Form Submit
  registerForm.addEventListener('submit', async (e) => {
    e.preventDefault();
    const rollNo = document.getElementById('reg-roll').value.trim();
    const name = document.getElementById('reg-name').value.trim();
    const department = document.getElementById('reg-dept').value;
    const year = parseInt(document.getElementById('reg-year').value) || 1;
    const email = document.getElementById('reg-email').value.trim();
    const contactNo = document.getElementById('reg-contact').value.trim();

    try {
      const res = await fetch(`${API_BASE}/api/students/register`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ rollNo, name, department, year, email, contactNo })
      });
      const result = await res.json();
      if (result.status === 'SUCCESS' && result.student) {
        allStudents.push(result.student);
        populateStudentSelect();
        setLoggedInStudent(result.student);
        authModal.classList.remove('active');
        registerForm.reset();
        showToast(`Account created! Welcome, ${result.student.name}!`, 'success');
        await loadCatalogItems();
        await loadStats();
      } else {
        showToast(result.message || 'Registration failed', 'error');
      }
    } catch (err) {
      showToast('Failed to register student', 'error');
    }
  });

  // Confirmation Modal Buttons for Cancel Listing
  document.getElementById('btn-confirm-cancel').addEventListener('click', () => {
    confirmModal.classList.remove('active');
    pendingCancelItemId = null;
  });

  document.getElementById('btn-confirm-proceed').addEventListener('click', async () => {
    if (!pendingCancelItemId) return;
    const itemId = pendingCancelItemId;
    confirmModal.classList.remove('active');
    pendingCancelItemId = null;

    try {
      const res = await fetch(`${API_BASE}/api/items/cancel-lend`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({
          itemId: itemId,
          ownerRollNo: activeStudent ? activeStudent.rollNo : ''
        })
      });

      const result = await res.json();
      if (result.status === 'SUCCESS') {
        showToast(result.message || 'Listing successfully cancelled', 'success');
        if (itemModal && itemModal.classList.contains('active')) {
          itemModal.classList.remove('active');
        }
        await refreshActiveStudentProfile();
        await loadCatalogItems();
        await loadStats();
        updateDrawerContent();
      } else {
        showToast(result.message || 'Failed to cancel listing', 'error');
      }
    } catch (err) {
      console.error('Cancel lending error:', err);
      showToast('Network error while cancelling listing', 'error');
    }
  });

  // Feedback Modal Controls & Star Ratings
  if (btnCloseFeedbackModal) {
    btnCloseFeedbackModal.addEventListener('click', () => {
      feedbackModal.classList.remove('active');
    });
  }

  const btnSkipFeedback = document.getElementById('btn-skip-feedback');
  if (btnSkipFeedback) {
    btnSkipFeedback.addEventListener('click', () => {
      submitReturnWithFeedback(5, '');
    });
  }

  const feedbackForm = document.getElementById('feedback-return-form');
  if (feedbackForm) {
    feedbackForm.addEventListener('submit', (e) => {
      e.preventDefault();
      const comment = document.getElementById('feedback-comment').value.trim();
      submitReturnWithFeedback(currentFeedbackRating, comment);
    });
  }

  // Star Rating button clicks
  const starBtns = document.querySelectorAll('#star-rating-wrap .star-btn');
  starBtns.forEach(btn => {
    btn.addEventListener('click', () => {
      const val = parseInt(btn.getAttribute('data-value')) || 5;
      updateStarRatingUI(val);
    });
    btn.addEventListener('mouseenter', () => {
      const val = parseInt(btn.getAttribute('data-value')) || 5;
      starBtns.forEach(b => {
        const v = parseInt(b.getAttribute('data-value'));
        if (v <= val) b.classList.add('active');
        else b.classList.remove('active');
      });
    });
  });
  const starWrap = document.getElementById('star-rating-wrap');
  if (starWrap) {
    starWrap.addEventListener('mouseleave', () => {
      updateStarRatingUI(currentFeedbackRating);
    });
  }

  // Wanted Board Filter Pills
  const reqFilterPills = document.querySelectorAll('#request-filter-pills .pill-btn');
  reqFilterPills.forEach(pill => {
    pill.addEventListener('click', () => {
      reqFilterPills.forEach(p => p.classList.remove('active'));
      pill.classList.add('active');
      currentRequestFilter = pill.getAttribute('data-filter') || 'all';
      renderRequests();
    });
  });

  // Fulfill Modal close controls
  const btnCloseFulfill = document.getElementById('btn-close-fulfill-modal');
  if (btnCloseFulfill) {
    btnCloseFulfill.addEventListener('click', () => {
      document.getElementById('fulfill-modal-overlay').classList.remove('active');
    });
  }

  // Close modals on overlay click
  [itemModal, lendModal, requestModal, statsModal, authModal, confirmModal, feedbackModal, fulfillModal, buyModal].forEach(overlay => {
    if (overlay) {
      overlay.addEventListener('click', (e) => {
        if (e.target === overlay) {
          overlay.classList.remove('active');
        }
      });
    }
  });
}

// Run on page load
document.addEventListener('DOMContentLoaded', init);
